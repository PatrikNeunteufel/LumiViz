/**
 ****************************************************************************************
 * @file   test_LoopKoerperLoch.cpp
 * @brief  Stufe 34k-5: Loecher in der Platte des Instrumentkoerpers
 *         (docs/audio/Instrumente_IC_Konzept.md §6, Entscheid Patrik S195: KQ7 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung.
 *
 * - Bauplan: Flaeche und Masse ohne die Loecher, die Pruefung (Loch ausserhalb, zwei Loecher
 *   beruehren sich, Stegpunkt im Loch), die Datei (Fassung 4, `holes` immer geschrieben,
 *   Fassung 3 ohne die Liste lesbar), der Abdruck.
 * - Netz, gegen die Schnittstelle (`netzgeber()`): im Loch kein Dreieck, die Flaeche stimmt,
 *   der Rand jedes Lochs als `lochkanten`, Leisten enden am Lochrand, zwei Laeufe gleich;
 *   ohne Loecher bleibt `lochkanten` leer.
 * - Platte: der Kreisring, aussen eingespannt, innen frei, gegen die geschlossene Loesung nach
 *   Kirchhoff (Bessel-Funktionen, Determinante 4×4 je Knotendurchmesserzahl n) — die Loesung
 *   selbst geprueft am Grenzfall der vollen Scheibe (λ² = 10,2158 / 21,26 / 34,88). Gegenprobe:
 *   mit gehaltenem Lochrand liegt die erste Mode weit hoeher.
 * - Abstrahlung: der Ring-Kolben auf der Achse gegen die geschlossene Loesung (Scheibe aussen
 *   minus Scheibe innen) — das Loch strahlt nicht.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/NetzCdt.hpp"
#include "InstrumentBody/Platte.hpp"
#include "InstrumentBody/Umsetzungen.hpp"
#include "loop/Koerper.hpp"
#include "loop/KoerperBauplan.hpp"

#include <Eigen/Dense>

#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <set>
#include <vector>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

std::vector<kp::Punkt2> vieleck(kp::Punkt2 mitte, double radius, int ecken)
{
    std::vector<kp::Punkt2> p;
    for (int i = 0; i < ecken; ++i)
    {
        const double w = 2.0 * std::numbers::pi * i / ecken;
        p.push_back({mitte.x + radius * std::cos(w), mitte.y + radius * std::sin(w)});
    }
    return p;
}

kp::Material aluminium()
{
    kp::Material m;
    m.name = "aluminium";
    m.art = kp::MaterialArt::Isotrop;
    m.dichte = 2700.0;
    m.ex = 70.0e9;
    m.nuxy = 0.3;
    return m;
}

kp::Bauplan lochPlan()
{
    kp::Bauplan plan;
    plan.titel = "Platte mit Loch";
    plan.materialien = {aluminium()};
    plan.platte.material = "aluminium";
    plan.platte.umriss = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 0.5}, {0.0, 0.5}};
    plan.platte.dicke = 0.004;
    plan.platte.loecher = {{{0.2, 0.1}, {0.4, 0.1}, {0.4, 0.3}, {0.2, 0.3}}};
    return plan;
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

// --- der Kreisring nach Kirchhoff ---------------------------------------------------
//
// W(r) cos nθ mit W = A J_n(kr) + B Y_n(kr) + C I_n(kr) + D K_n(kr), Aussenradius 1, Innenradius β.
// Aussen eingespannt: W = 0, W' = 0. Innen frei: M_r = 0, V_r = 0 mit
//   M_r ∝ W'' + ν (W'/r − n² W/r²)
//   V_r ∝ (∇²W)' − (1 − ν) n² (W'/r² − W/r³)          (Leissa, Vibration of Plates, Gl. 1.11–1.13)
// ∇²f = s k² f mit s = −1 fuer J, Y und s = +1 fuer I, K; f'' aus der Bessel-Gleichung.

struct Bessel
{
    double f = 0.0, f1 = 0.0, f2 = 0.0;  ///< Wert, erste und zweite Ableitung nach x
    double s = 0.0;
};

Bessel bessel(int art, int n, double x)
{
    Bessel b;
    const double nx = n / x;
    switch (art)
    {
        case 0:
            b.f = std::cyl_bessel_j(static_cast<double>(n), x);
            b.f1 = nx * b.f - std::cyl_bessel_j(n + 1.0, x);
            b.s = -1.0;
            break;
        case 1:
            b.f = std::cyl_neumann(static_cast<double>(n), x);
            b.f1 = nx * b.f - std::cyl_neumann(n + 1.0, x);
            b.s = -1.0;
            break;
        case 2:
            b.f = std::cyl_bessel_i(static_cast<double>(n), x);
            b.f1 = nx * b.f + std::cyl_bessel_i(n + 1.0, x);
            b.s = 1.0;
            break;
        default:
            b.f = std::cyl_bessel_k(static_cast<double>(n), x);
            b.f1 = nx * b.f - std::cyl_bessel_k(n + 1.0, x);
            b.s = 1.0;
            break;
    }
    b.f2 = -b.f1 / x + (static_cast<double>(n) * n / (x * x) + b.s) * b.f;
    return b;
}

double ringDeterminante(int n, double lambda, double beta, double nu)
{
    Eigen::Matrix4d m;
    const double n2 = static_cast<double>(n) * n;
    for (int art = 0; art < 4; ++art)
    {
        const Bessel a = bessel(art, n, lambda);
        const double x = lambda * beta;
        const Bessel i = bessel(art, n, x);
        m(0, art) = a.f;
        m(1, art) = a.f1;
        m(2, art) = i.f2 + nu * (i.f1 / x - n2 * i.f / (x * x));
        m(3, art) = i.s * i.f1 - (1.0 - nu) * n2 * (i.f1 / (x * x) - i.f / (x * x * x));
    }
    // die Spalten laufen weit auseinander (I waechst, K faellt): je Spalte normieren
    for (int art = 0; art < 4; ++art) m.col(art).normalize();
    return m.determinant();
}

/// Die kleinste Wurzel λ = k·a der Determinante oberhalb von `ab`.
double ringLambda(int n, double beta, double nu, double ab = 0.5)
{
    const double schritt = 0.005;
    double x0 = ab, d0 = ringDeterminante(n, x0, beta, nu);
    for (double x1 = ab + schritt; x1 < 20.0; x1 += schritt)
    {
        const double d1 = ringDeterminante(n, x1, beta, nu);
        if ((d0 < 0.0) != (d1 < 0.0))
        {
            double lo = x0, hi = x1, dlo = d0;
            for (int i = 0; i < 80; ++i)
            {
                const double mitte = (lo + hi) / 2.0, dm = ringDeterminante(n, mitte, beta, nu);
                if ((dm < 0.0) == (dlo < 0.0))
                {
                    lo = mitte;
                    dlo = dm;
                }
                else hi = mitte;
            }
            return (lo + hi) / 2.0;
        }
        x0 = x1;
        d0 = d1;
    }
    return 0.0;
}

}  // namespace

TEST_CASE("Loop Koerper 34k-5 Loch Bauplan")
{
    const kp::Bauplan plan = lochPlan();
    CHECK(kp::pruefe(plan).empty());
    CHECK(kp::flaeche(plan.platte) == doctest::Approx(0.5 - 0.04));
    CHECK(kp::imLoch(plan.platte.loecher, {0.3, 0.2}));
    CHECK_FALSE(kp::imLoch(plan.platte.loecher, {0.6, 0.2}));
    // die Masse ohne das Loch
    kp::Bauplan ohne = plan;
    ohne.platte.loecher.clear();
    CHECK(kp::masse(ohne) == doctest::Approx(0.5 * 0.004 * 2700.0).epsilon(1e-9));
    CHECK(kp::masse(plan) == doctest::Approx((0.5 - 0.04) * 0.004 * 2700.0).epsilon(1e-9));

    const auto befundMit = [](const kp::Bauplan& p, const char* stueck) {
        for (const std::string& b : kp::pruefe(p))
            if (b.find(stueck) != std::string::npos) return true;
        return false;
    };
    {
        kp::Bauplan p = plan;  // ragt ueber den Umriss
        p.platte.loecher = {{{0.9, 0.1}, {1.1, 0.1}, {1.1, 0.3}, {0.9, 0.3}}};
        CHECK(befundMit(p, "inside the outline"));
    }
    {
        kp::Bauplan p = plan;  // alle Ecken innen, aber eine Kante kreuzt den Umriss (einspringende Ecke)
        p.platte.umriss = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 0.5}, {0.5, 0.2}, {0.0, 0.5}};
        p.platte.loecher = {{{0.3, 0.1}, {0.7, 0.1}, {0.7, 0.3}, {0.3, 0.3}}};
        CHECK(befundMit(p, "inside the outline"));
    }
    {
        kp::Bauplan p = plan;  // zwei Loecher uebereinander
        p.platte.loecher.push_back({{0.3, 0.2}, {0.6, 0.2}, {0.6, 0.4}, {0.3, 0.4}});
        CHECK(befundMit(p, "touches hole"));
    }
    {
        kp::Bauplan p = plan;  // ein Loch ganz im anderen
        p.platte.loecher.push_back({{0.25, 0.15}, {0.35, 0.15}, {0.35, 0.25}, {0.25, 0.25}});
        CHECK(befundMit(p, "touches hole"));
    }
    {
        kp::Bauplan p = plan;
        p.platte.loecher = {{{0.2, 0.1}, {0.4, 0.1}}};
        CHECK(befundMit(p, "three points"));
    }
    {
        kp::Bauplan p = plan;
        p.stegpunkte = {{60, {0.3, 0.2}, "Test"}};
        CHECK(befundMit(p, "lies in a hole"));
    }
    {
        kp::Bauplan p = plan;  // zwei getrennte Loecher sind in Ordnung
        p.platte.loecher.push_back(vieleck({0.7, 0.25}, 0.1, 24));
        CHECK(kp::pruefe(p).empty());
    }
}

TEST_CASE("Loop Koerper 34k-5 Loch Datei")
{
    const kp::Bauplan plan = lochPlan();
    const QJsonObject json = alsJson(plan);
    CHECK(json.value(QStringLiteral("version")).toInt() == 4);
    const QJsonObject platte = json.value(QStringLiteral("plate")).toObject();
    REQUIRE(platte.value(QStringLiteral("holes")).toArray().size() == 1);
    CHECK(platte.value(QStringLiteral("holes")).toArray()[0].toArray().size() == 4);
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == plan);

    // immer geschrieben, auch leer
    kp::Bauplan ohne = plan;
    ohne.platte.loecher.clear();
    const QJsonObject jsonOhne = alsJson(ohne);
    CHECK(jsonOhne.value(QStringLiteral("plate")).toObject().contains(QStringLiteral("holes")));
    CHECK(jsonOhne.value(QStringLiteral("plate")).toObject().value(QStringLiteral("holes")).toArray().isEmpty());

    // Fassung 3 kennt die Liste nicht: lesbar, ohne Loecher
    QJsonObject drei = jsonOhne;
    QJsonObject platteDrei = drei.value(QStringLiteral("plate")).toObject();
    platteDrei.remove(QStringLiteral("holes"));
    drei[QStringLiteral("plate")] = platteDrei;
    drei[QStringLiteral("version")] = 3;
    kp::Bauplan alt;
    REQUIRE(ausJson(drei, alt));
    CHECK(alt == ohne);

    // ein kaputter Punkt in einem Loch
    QJsonObject kaputt = json;
    QJsonObject kaputtePlatte = platte;
    // mit append gebaut: geschachtelte Klammern liest MSVC als Kopie, clang als Liste in der Liste
    QJsonArray kaputterPunkt, kaputtesLoch, kaputteLoecher;
    kaputterPunkt.append(0.0);
    kaputtesLoch.append(kaputterPunkt);
    kaputteLoecher.append(kaputtesLoch);
    kaputtePlatte[QStringLiteral("holes")] = kaputteLoecher;
    kaputt[QStringLiteral("plate")] = kaputtePlatte;
    kp::Bauplan bleibt;
    CHECK_FALSE(ausJson(kaputt, bleibt));

    // ein Loch aendert den Abdruck
    CHECK(bauplanAbdruck(plan) != bauplanAbdruck(ohne));
}

TEST_CASE("Loop Koerper 34k-5 Loch Netz")
{
    kp::Bauplan plan = lochPlan();
    plan.platte.loecher.push_back(vieleck({0.7, 0.25}, 0.1, 24));
    // eine Rippe quer durch beide Loecher, mit Punkten in den Loechern
    plan.rippen.push_back({"rib", "aluminium", {{0.05, 0.2}, {0.3, 0.2}, {0.55, 0.2}, {0.7, 0.2}, {0.95, 0.2}},
                           {0.01, 0.01, 0.01, 0.01, 0.01}, {0.02, 0.02, 0.02, 0.02, 0.02}});
    REQUIRE(kp::pruefe(plan).empty());
    kp::NetzAuftrag a = kp::netzAuftrag(plan, 2000.0);
    a.kante = 0.04;
    REQUIRE(a.loecher.size() == 2);
    // keine Zwangslinie hat einen Punkt in einem Loch (auf dem Rand darf er liegen)
    for (const auto& linie : a.zwangslinien)
        for (std::size_t k = 0; k + 1 < linie.size(); ++k)
            CHECK_FALSE(kp::imLoch(a.loecher, {(linie[k].x + linie[k + 1].x) / 2.0, (linie[k].y + linie[k + 1].y) / 2.0}));
    CHECK(a.zwangslinien.size() == 6);  // je Rand der Rippe drei Stuecke

    const double soll = kp::flaeche(plan.platte);
    for (const auto& geber : kp::netzgeber())
    {
        CAPTURE(geber->name());
        const kp::Netz n = geber->vernetze(a);
        REQUIRE_MESSAGE(n.fehler.empty(), n.fehler);
        CHECK(kp::flaeche(n) == doctest::Approx(soll).epsilon(1e-12));
        int imLoch = 0, falschHerum = 0;
        for (const auto& d : n.dreiecke)
        {
            if (kp::flaeche(n, d) <= 0.0) ++falschHerum;
            const kp::Punkt2 m{(n.knoten[d[0]].x + n.knoten[d[1]].x + n.knoten[d[2]].x) / 3.0,
                               (n.knoten[d[0]].y + n.knoten[d[1]].y + n.knoten[d[2]].y) / 3.0};
            if (kp::imLoch(a.loecher, m)) ++imLoch;
        }
        CHECK(imLoch == 0);
        CHECK(falschHerum == 0);
        // der Rand jedes Lochs liegt ganz auf Zwangskanten, und die Lochkanten fuellen ihn genau
        double umfang = 0.0;
        for (const auto& loch : a.loecher)
        {
            for (std::size_t k = 0; k < loch.size(); ++k)
            {
                const kp::Punkt2 p = loch[k], q = loch[(k + 1) % loch.size()];
                CHECK(kp::aufZwangskanten(n, p, q, 1e-7));
                umfang += std::hypot(q.x - p.x, q.y - p.y);
            }
        }
        double lochLaenge = 0.0;
        std::set<std::array<int, 2>> zwang(n.zwangskanten.begin(), n.zwangskanten.end());
        for (const auto& k : n.lochkanten)
        {
            lochLaenge += std::hypot(n.knoten[k[1]].x - n.knoten[k[0]].x, n.knoten[k[1]].y - n.knoten[k[0]].y);
            CHECK(zwang.count(k) == 1);
        }
        CHECK(lochLaenge == doctest::Approx(umfang).epsilon(1e-9));
        CHECK(std::is_sorted(n.lochkanten.begin(), n.lochkanten.end()));
        MESSAGE(geber->name(), ": ", n.knoten.size(), " Knoten, ", n.dreiecke.size(), " Dreiecke, ", n.lochkanten.size(), " Lochkanten");
        CHECK(geber->vernetze(a) == n);

        // ohne Loecher: keine Lochkanten, die volle Flaeche
        kp::NetzAuftrag voll = a;
        voll.loecher.clear();
        const kp::Netz nv = geber->vernetze(voll);
        REQUIRE(nv.fehler.empty());
        CHECK(nv.lochkanten.empty());
        CHECK(kp::flaeche(nv) == doctest::Approx(0.5).epsilon(1e-12));

        // ein Loch mit zwei Punkten gibt es nicht
        kp::NetzAuftrag kaputt = a;
        kaputt.loecher = {{{0.2, 0.2}, {0.3, 0.2}}};
        CHECK_FALSE(geber->vernetze(kaputt).fehler.empty());
    }
}

TEST_CASE("Loop Koerper 34k-5 Loch Kreisring gegen Kirchhoff")
{
    const double nu = 0.3;
    // die geschlossene Loesung am Grenzfall: fast kein Loch = die eingespannte volle Scheibe
    // (Leissa Tab. 2.1: λ² = 10,2158; 21,26; 34,88 fuer n = 0, 1, 2)
    CHECK(std::pow(ringLambda(0, 1e-4, nu), 2) == doctest::Approx(10.2158).epsilon(2e-4));
    CHECK(std::pow(ringLambda(1, 1e-4, nu), 2) == doctest::Approx(21.26).epsilon(1e-3));
    CHECK(std::pow(ringLambda(2, 1e-4, nu), 2) == doctest::Approx(34.88).epsilon(1e-3));

    const kp::Material mat = aluminium();
    const double h = 0.003;
    const std::vector<kp::Punkt2> aussen = vieleck({0.0, 0.0}, 0.2, 32), innen = vieleck({0.0, 0.0}, 0.06, 16);
    const double ra = std::sqrt(kp::flaeche(aussen) / std::numbers::pi), ri = std::sqrt(kp::flaeche(innen) / std::numbers::pi);  // flaechengleich
    const double beta = ri / ra;
    const double d = mat.ex * h * h * h / (12.0 * (1.0 - nu * nu));
    std::vector<double> soll;
    for (const int n : {0, 1, 2})
    {
        const double lambda = ringLambda(n, beta, nu);
        REQUIRE(lambda > 0.0);
        const double f = lambda * lambda / (2.0 * std::numbers::pi * ra * ra) * std::sqrt(d / (mat.dichte * h));
        MESSAGE("Kreisring beta = ", beta, ", n = ", n, ": lambda^2 = ", lambda * lambda, ", f = ", f, " Hz");
        soll.push_back(f);
        if (n > 0) soll.push_back(f);  // cos und sin
    }
    std::sort(soll.begin(), soll.end());

    kp::NetzAuftrag a;
    a.umriss = aussen;
    a.loecher = {innen};
    a.kante = 0.04;
    const auto steife = [&](kp::Punkt2) {
        kp::Steife s = kp::gedreht(mat, 0.0);
        s.h = h;
        return s;
    };
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(a);
    REQUIRE_MESSAGE(netz.fehler.empty(), netz.fehler);
    REQUIRE_FALSE(netz.lochkanten.empty());
    const kp::PlattenSystem s = kp::baueSystem(netz, kp::LagrangeDreieck(3), steife, kp::Auflager::Eingespannt);
    REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
    CHECK(s.masse == doctest::Approx((kp::flaeche(aussen) - kp::flaeche(innen)) * h * mat.dichte).epsilon(1e-9));
    const std::vector<double> f = frequenzen(s, 5);
    REQUIRE(f.size() == soll.size());
    for (std::size_t i = 0; i < soll.size(); ++i)
    {
        CAPTURE(i);
        MESSAGE("Mode ", i + 1, ": gerechnet ", f[i], " Hz, Kirchhoff ", soll[i], " Hz, Abstand ", (f[i] / soll[i] - 1.0) * 100.0, " %");
        CHECK(f[i] == doctest::Approx(soll[i]).epsilon(0.005));  // gemessen S196: bis 0,09 %
    }

    // Gegenprobe: der Lochrand gehalten (wie jeder Rand vor KQ7 a) — die erste Mode liegt weit hoeher
    kp::Netz gehalten = netz;
    gehalten.lochkanten.clear();
    const kp::PlattenSystem sg = kp::baueSystem(gehalten, kp::LagrangeDreieck(3), steife, kp::Auflager::Eingespannt);
    CHECK(sg.k.n < s.k.n);
    const std::vector<double> fg = frequenzen(sg, 1);
    MESSAGE("Lochrand gehalten: erste Mode ", fg[0], " Hz gegen ", f[0], " Hz");
    CHECK(fg[0] > 2.0 * f[0]);
}

TEST_CASE("Loop Koerper 34k-5 Loch Ring-Kolben")
{
    // ein Ring, der sich ganz mit w = 1 bewegt: auf der Achse die Scheibe aussen minus die Scheibe innen,
    // je ∫ e^{−ikR}/R dS = 2π (e^{−ik√(z²+a²)} − e^{−ikz}) / (−ik)
    const std::vector<kp::Punkt2> aussen = vieleck({0.0, 0.0}, 0.1, 256), innen = vieleck({0.0, 0.0}, 0.04, 128);
    const double ra = std::sqrt(kp::flaeche(aussen) / std::numbers::pi), ri = std::sqrt(kp::flaeche(innen) / std::numbers::pi);
    kp::NetzAuftrag a;
    a.umriss = aussen;
    a.loecher = {innen};
    a.kante = 0.015;
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(a);
    REQUIRE_MESSAGE(netz.fehler.empty(), netz.fehler);
    const kp::Material mat = aluminium();
    const int k = 2;
    const kp::PlattenSystem s = kp::baueSystem(netz, kp::LagrangeDreieck(k),
                                               [&](kp::Punkt2) {
                                                   kp::Steife st = kp::gedreht(mat, 0.0);
                                                   st.h = 0.003;
                                                   return st;
                                               },
                                               kp::Auflager::Frei);
    REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
    const kp::Luft luft;
    std::vector<kp::Hoerort> orte;
    for (const double z : {0.05, 0.2, 1.0}) orte.push_back({"z" + std::to_string(z), {0.0, 0.0}, z, "Achse"});
    const kp::Auswertung auswertung(s, kp::LagrangeDreieck(k), {}, orte, luft);
    kp::Mode mode;
    mode.form.assign(static_cast<std::size_t>(s.k.n), 0.0);
    for (std::size_t q = 0; q < s.knoten.size(); ++q)
        if (const int g = s.frei[3 * q]; g >= 0) mode.form[static_cast<std::size_t>(g)] = 1.0;
    const std::complex<double> i(0.0, 1.0);
    for (const double f : {200.0, 2000.0, 8000.0})
    {
        mode.frequenzHz = f;
        const kp::ModeImKoerper m = auswertung.werte(mode);
        REQUIRE(m.abstrahlung.size() == orte.size());
        const double wz = 2.0 * std::numbers::pi * f / luft.schall;
        for (std::size_t o = 0; o < orte.size(); ++o)
        {
            const double z = orte[o].hoehe;
            const auto scheibe = [&](double r) {
                return 2.0 * std::numbers::pi * (std::exp(-i * wz * std::sqrt(z * z + r * r)) - std::exp(-i * wz * z)) / (-i * wz);
            };
            const std::complex<double> soll = luft.dichte / (2.0 * std::numbers::pi) * (scheibe(ra) - scheibe(ri));
            CAPTURE(f);
            CAPTURE(z);
            CHECK(std::abs(m.abstrahlung[o] - soll) < 3e-3 * std::abs(soll));
        }
    }
}
