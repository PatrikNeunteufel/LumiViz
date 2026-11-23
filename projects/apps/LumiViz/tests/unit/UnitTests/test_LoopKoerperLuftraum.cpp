/**
 ****************************************************************************************
 * @file   test_LoopKoerperLuftraum.cpp
 * @brief  Stufe 34k-5: der Luftraum hinter der Platte des Instrumentkoerpers
 *         (docs/audio/Instrumente_IC_Konzept.md §6: F-K1 a, Entscheid Patrik S196: KQ8 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung.
 *
 * - `eigenRangEins` (Diagonale plus Rang 1) gegen den dichten Loeser von Eigen: Eigenwerte, Rest,
 *   Orthonormalitaet — auch mit einem Eintrag ohne Kopplung und mit zwei gleichen Diagonalwerten.
 * - Die geschlossenen Loesungen: Helmholtz (starre Wand), Kasten (kein Loch: ω² = ω_p² + α a²), zwei
 *   gekoppelte Schwinger (eine Mode der Platte und die Luft im Loch), die Phase von Platte und Luft
 *   an der unteren und der oberen Mode.
 * - Der Hohlraum aus dem Bauplan: Laenge und Masse der Luft im Loch, zwei Loecher parallel, die Datei
 *   (`cavity`, immer geschrieben; Fassung 3 ohne den Block: aus), die Pruefung.
 * - An einer gerechneten Platte mit Loch: `Auswertung::volumen`; ein riesiger Hohlraum laesst die
 *   Moden der Platte, wie sie sind; ein kleiner hebt die Moden mit Volumen und laesst die ohne; die
 *   Summe der Eigenwerte stimmt (Spur).
 * - Die mitgelieferte Oud (KQ10 a, neu gesucht nach KQ42 a–KQ45 a; vom Arbeitsordner aus): der Bauplan mit
 *   drei Loechern samt Rosetten, Steg, Hohlraum und sechs Stegpunkten; der Koerper mit den ersten fuenf Moden
 *   des Instruments ohne den Aufnehmer der Messung (mit ihm treffen sie die Spitzen von Abdallah u. a. 2026,
 *   Abb. 6: 139 / 203 / 280 / 362 / 385 Hz innerhalb von 3 %), Luft- und Deckenmode bewegen den Steg, die
 *   Hauptspitze der Abstrahlung liegt beim 1,5- bis 2,0-Fachen der Luftmode (KQ41 a).
 * - Die mitgelieferte Laute (KQ11 a, KQ12 a): die Form der Rauwolf-Laute im Massstab 625/677,4, acht
 *   Stegpunkte; die Stegadmittanz zwischen Chor 1 und 2 mit der Last der gedaempften Saiten trifft die
 *   Spitzen der gemessenen Kurve (Woodhouse 2021, Abb. 1: 132 / 277 / 341 / 476 Hz) in Lage (5 %) und
 *   Hoehe (6 dB).
 * - Das mitgelieferte Cembalo (KQ13 a, KQ15 a, KQ16 a): der Boden des Dulcken 1755 (Bader u. a. 2025) mit
 *   der gemessenen Dicke als Raster, 61 Stegpunkte (die neun Tasten ausserhalb des Dulcken am Rand),
 *   Helmholtz bei starrer Wand 37 Hz, die Daempfung aus der gemessenen Abklingzeit; die gekoppelte Mode
 *   der Luft als bekannte Abweichung festgehalten.
 * - Die mitgelieferte Harfe (KQ19 a–KQ22 a, KQ46 a, KQ48 a, KQ50 a–KQ54 a; seit S215 die Luft als Rohr, KQ57 a,
 *   KQ59 a–KQ64 a): der Bauplan mit Drehfeder an den Bassseiten und den Saiten als Federn an den Oesen; die Lagen der
 *   besaiteten Harfe gegen Bell besaitet (KQ57 a); die Verschiebung beim Besaiten gegen den Koerper ohne Saiten. Der
 *   Fall rechnet die Platte bis 1,5 kHz selbst (die Laengsmoden des Rohrs bis 5 kHz): ohne Saiten die Bodenmode bei
 *   verschlossenen Oeffnungen gegen die Helmholtz-Mode (KQ60 a, 3 %), Lage und Hoehe der Baeuche gegen Bell 1988
 *   (KQ31-2 a) und das Profil von S(1,1)- (KQ60 a) mit den bekannten Abweichungen; die Abstrahlung an den Saiten
 *   33–22 an der besaiteten Platte mit 16 Hoerorten nach KQ36 a, die Saiten als Daempfer, bezogen auf den Median
 *   (KQ54 a, KQ60 a). Zusammen rund 45 s.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Luftraum.hpp"
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
#include <cstdint>
#include <numbers>
#include <utility>
#include <vector>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

constexpr double kPi = std::numbers::pi;

std::vector<kp::Punkt2> vieleck(kp::Punkt2 mitte, double radius, int ecken)
{
    std::vector<kp::Punkt2> p;
    for (int i = 0; i < ecken; ++i)
    {
        const double w = 2.0 * kPi * i / ecken;
        p.push_back({mitte.x + radius * std::cos(w), mitte.y + radius * std::sin(w)});
    }
    return p;
}

kp::Material fichte()
{
    kp::Material m;
    m.name = "spruce";
    m.dichte = 400.0;
    m.ex = 11.0e9;
    m.ey = 0.65e9;
    m.gxy = 0.66e9;
    m.gxz = 1.2e9;
    m.gyz = 0.042e9;
    m.nuxy = 0.26;
    return m;
}

/// Eine Decke mit einem Loch: Rechteck 0,4 m × 0,3 m, 3 mm Fichte, Loch Ø 8 cm neben der Mitte.
kp::Bauplan decke()
{
    kp::Bauplan plan;
    plan.titel = "Decke mit Loch";
    plan.materialien = {fichte()};
    plan.platte.material = "spruce";
    plan.platte.umriss = {{0.0, 0.0}, {0.4, 0.0}, {0.4, 0.3}, {0.0, 0.3}};
    plan.platte.dicke = 0.003;
    plan.platte.loecher = {vieleck({0.27, 0.15}, 0.04, 24)};
    plan.stegpunkte = {{60, {0.12, 0.15}, "Test"}};
    plan.hoerorte = {{"front", {0.2, 0.15}, 0.5, "Test"}};
    return plan;
}

struct Gerechnet
{
    kp::PlattenSystem system;
    std::vector<kp::Mode> moden;  ///< M-orthonormal, aufsteigend
};

/// Die ersten Moden einer Platte (dicht mit Eigen; nur fuer kleine Systeme).
Gerechnet rechne(const kp::Bauplan& plan, const kp::Plattenelement& element, double kante, int wieviele)
{
    Gerechnet g;
    kp::NetzAuftrag a = kp::netzAuftrag(plan, 1000.0);
    a.kante = kante;
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(a);
    REQUIRE_MESSAGE(netz.fehler.empty(), netz.fehler);
    g.system = kp::baueSystem(netz, element, plan);
    REQUIRE_MESSAGE(g.system.fehler.empty(), g.system.fehler);
    const int n = g.system.k.n;
    Eigen::MatrixXd k = Eigen::MatrixXd::Zero(n, n), m = Eigen::MatrixXd::Zero(n, n);
    for (int i = 0; i < n; ++i)
    {
        for (std::int64_t e = g.system.k.start[i]; e < g.system.k.start[i + 1]; ++e)
        {
            const int j = g.system.k.spalten[static_cast<std::size_t>(e)];
            k(i, j) = k(j, i) = g.system.k.werte[static_cast<std::size_t>(e)];
            m(i, j) = m(j, i) = g.system.m.werte[static_cast<std::size_t>(e)];
        }
    }
    const Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::MatrixXd> loeser(k, m);
    for (int i = 0; i < std::min(wieviele, n); ++i)
    {
        kp::Mode mode;
        mode.frequenzHz = std::sqrt(std::max(0.0, loeser.eigenvalues()(i))) / (2.0 * kPi);
        mode.form.assign(loeser.eigenvectors().col(i).data(), loeser.eigenvectors().col(i).data() + n);
        g.moden.push_back(std::move(mode));
    }
    return g;
}

}  // namespace

TEST_CASE("Loop Koerper 34k-5 Luftraum Rang eins gegen Eigen")
{
    // feste Zufallsfolge (LCG), damit beide Compiler dasselbe rechnen
    std::uint64_t zustand = 12345;
    const auto zufall = [&] {
        zustand = zustand * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<double>(zustand >> 11) / 9007199254740992.0;
    };
    const int n = 40;
    std::vector<double> d(n), z(n);
    for (int i = 0; i < n; ++i)
    {
        d[i] = 1.0e4 + 4.0e6 * zufall();
        z[i] = 2.0 * zufall() - 1.0;
    }
    d[0] = 0.0;      // die Luft im Loch
    z[7] = 0.0;      // eine Mode ohne Volumen
    d[20] = d[11];   // zwei gleiche Diagonalwerte, beide gekoppelt
    const double alpha = 3.0e5;

    const kp::RangEins e = kp::eigenRangEins(d, z, alpha);
    REQUIRE(e.werte.size() == static_cast<std::size_t>(n));

    Eigen::MatrixXd a = Eigen::MatrixXd::Zero(n, n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) a(i, j) = (i == j ? d[i] : 0.0) + alpha * z[i] * z[j];
    const Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> dicht(a);
    const double skala = dicht.eigenvalues()(n - 1);
    double groessterRest = 0.0;
    for (int m = 0; m < n; ++m)
    {
        CAPTURE(m);
        CHECK(std::abs(e.werte[m] - dicht.eigenvalues()(m)) <= 1e-11 * skala);
        const Eigen::Map<const Eigen::VectorXd> v(e.vektoren[m].data(), n);
        CHECK(v.norm() == doctest::Approx(1.0).epsilon(1e-12));
        groessterRest = std::max(groessterRest, (a * v - e.werte[m] * v).norm() / skala);
        for (int l = 0; l < m; ++l)
        {
            const Eigen::Map<const Eigen::VectorXd> u(e.vektoren[l].data(), n);
            CHECK(std::abs(u.dot(v)) < 1e-8);
        }
    }
    MESSAGE("groesster Rest ", groessterRest);
    CHECK(groessterRest < 1e-10);
    // die Mode ohne Volumen bleibt, wie sie ist
    bool gefunden = false;
    for (int m = 0; m < n; ++m)
        if (e.werte[m] == d[7] && std::abs(std::abs(e.vektoren[m][7]) - 1.0) < 1e-15) gefunden = true;
    CHECK(gefunden);
    // die Spur: Σλ = Σd + α Σz²
    double spur = 0.0, soll = 0.0;
    for (int i = 0; i < n; ++i)
    {
        spur += e.werte[i];
        soll += d[i] + alpha * z[i] * z[i];
    }
    CHECK(spur == doctest::Approx(soll).epsilon(1e-12));

    // leer und falsche Laengen
    CHECK(kp::eigenRangEins({}, {}, 1.0).werte.empty());
    CHECK(kp::eigenRangEins({1.0, 2.0}, {1.0}, 1.0).werte.empty());
}

TEST_CASE("Loop Koerper 34k-5 Luftraum geschlossene Loesungen")
{
    kp::LuftraumGroessen g;
    const double rho = 1.2, c = 343.0, volumen = 0.012;
    g.alpha = rho * c * c / volumen;
    const double flaeche = 5.0e-3, laenge = 0.07;
    g.akustischeMasse = rho * laenge / flaeche;
    g.loecher = {{{0.0, 0.0}, flaeche, laenge, 1.0}};
    const double fh = c / (2.0 * kPi) * std::sqrt(flaeche / (volumen * laenge));
    CHECK(g.helmholtzHz() == doctest::Approx(fh).epsilon(1e-12));

    // starre Wand (kein Volumen der Mode): die Luft schwingt bei Helmholtz, die Platte bleibt
    {
        const auto m = kp::koppleLuftraum({200.0}, {0.0}, g);
        REQUIRE(m.size() == 2);
        CHECK(m[0].frequenzHz == doctest::Approx(fh).epsilon(1e-10));
        CHECK(m[0].luftAnteil == doctest::Approx(1.0));
        CHECK(m[1].frequenzHz == doctest::Approx(200.0).epsilon(1e-12));
        CHECK(m[1].luftAnteil == doctest::Approx(0.0));
        CHECK(std::abs(m[1].platte[0]) == doctest::Approx(1.0));
    }
    // Kasten ohne Loch: die Luft ist eine Feder mehr, ω² = ω_p² + α a²
    {
        kp::LuftraumGroessen zu = g;
        zu.akustischeMasse = 0.0;
        zu.loecher.clear();
        const double a = 0.02;  // m²/√kg
        const auto m = kp::koppleLuftraum({150.0, 300.0}, {a, 0.0}, zu);
        REQUIRE(m.size() == 2);
        const double soll = std::sqrt(std::pow(2.0 * kPi * 150.0, 2) + zu.alpha * a * a) / (2.0 * kPi);
        CHECK(m[0].frequenzHz == doctest::Approx(soll).epsilon(1e-10));
        CHECK(m[0].oeffnung == 0.0);
        CHECK(m[1].frequenzHz == doctest::Approx(300.0).epsilon(1e-12));
    }
    // zwei gekoppelte Schwinger: eine Mode der Platte und die Luft im Loch
    {
        const double fp = 180.0, a = 0.03;
        const double kp2 = std::pow(2.0 * kPi * fp, 2) + g.alpha * a * a, kh2 = g.alpha / g.akustischeMasse;
        const double wurzel = std::sqrt(std::pow(kp2 - kh2, 2) + 4.0 * g.alpha * g.alpha * a * a / g.akustischeMasse);
        const double unten = std::sqrt((kp2 + kh2 - wurzel) / 2.0) / (2.0 * kPi), oben = std::sqrt((kp2 + kh2 + wurzel) / 2.0) / (2.0 * kPi);
        const auto m = kp::koppleLuftraum({fp}, {a}, g);
        REQUIRE(m.size() == 2);
        CHECK(m[0].frequenzHz == doctest::Approx(unten).epsilon(1e-10));
        CHECK(m[1].frequenzHz == doctest::Approx(oben).epsilon(1e-10));
        MESSAGE("Platte ", fp, " Hz, Helmholtz ", fh, " Hz -> gekoppelt ", m[0].frequenzHz, " und ", m[1].frequenzHz, " Hz");
        // die untere liegt unter beiden, die obere ueber beiden
        CHECK(m[0].frequenzHz < std::min(fp, fh));
        CHECK(m[1].frequenzHz > std::max(fp, fh));
        // an der unteren Mode gehen Platte und Luft gegeneinander (tritt die Platte heraus, stroemt Luft hinein),
        // an der oberen miteinander
        CHECK(m[0].oeffnung * (a * m[0].platte[0]) < 0.0);
        CHECK(m[1].oeffnung * (a * m[1].platte[0]) > 0.0);
        CHECK(m[0].luftAnteil + m[1].luftAnteil == doctest::Approx(1.0).epsilon(1e-12));
    }
    // ohne Hohlraum keine Kopplung
    CHECK(kp::koppleLuftraum({100.0}, {0.01}, kp::LuftraumGroessen{}).empty());
}

TEST_CASE("Loop Koerper 34k-5 Luftraum Bauplan und Datei")
{
    kp::Bauplan plan = decke();
    CHECK_FALSE(plan.hohlraum.an());
    CHECK(kp::luftraumGroessen(plan, 1.2, 343.0).alpha == 0.0);
    plan.hohlraum = {0.012, 1.7, 25.0, "Test"};
    REQUIRE(kp::pruefe(plan).empty());

    const kp::LuftraumGroessen g = kp::luftraumGroessen(plan, 1.2, 343.0);
    REQUIRE(g.loecher.size() == 1);
    const double flaeche = kp::flaeche(plan.platte.loecher[0]);
    const double radius = std::sqrt(flaeche / kPi);
    CHECK(g.loecher[0].mitte.x == doctest::Approx(0.27).epsilon(1e-9));
    CHECK(g.loecher[0].mitte.y == doctest::Approx(0.15).epsilon(1e-9));
    CHECK(g.loecher[0].laenge == doctest::Approx(0.003 + 1.7 * radius).epsilon(1e-12));
    CHECK(g.loecher[0].anteil == doctest::Approx(1.0));
    CHECK(g.helmholtzHz() == doctest::Approx(343.0 / (2.0 * kPi) * std::sqrt(flaeche / (0.012 * (0.003 + 1.7 * radius)))).epsilon(1e-12));

    // zwei gleiche Loecher liegen parallel: die halbe Masse, Helmholtz um √2 hoeher, der Fluss je zur Haelfte
    kp::Bauplan zwei = plan;
    zwei.platte.loecher.push_back(vieleck({0.10, 0.08}, 0.04, 24));
    REQUIRE(kp::pruefe(zwei).empty());
    const kp::LuftraumGroessen g2 = kp::luftraumGroessen(zwei, 1.2, 343.0);
    CHECK(g2.akustischeMasse == doctest::Approx(g.akustischeMasse / 2.0).epsilon(1e-9));
    CHECK(g2.helmholtzHz() == doctest::Approx(g.helmholtzHz() * std::sqrt(2.0)).epsilon(1e-9));
    CHECK(g2.loecher[0].anteil == doctest::Approx(0.5).epsilon(1e-9));

    // ein Hohlraum ohne Loch ist zu
    kp::Bauplan zu = plan;
    zu.platte.loecher.clear();
    CHECK(kp::luftraumGroessen(zu, 1.2, 343.0).akustischeMasse == 0.0);
    CHECK(kp::luftraumGroessen(zu, 1.2, 343.0).alpha > 0.0);

    // die Datei: `cavity` immer geschrieben, Hin- und Rueckweg, Fassung 3 ohne den Block
    const QJsonObject json = alsJson(plan);
    const QJsonObject c = json.value(QStringLiteral("cavity")).toObject();
    for (const char* feld : {"volume", "endCorrection", "q", "source"}) CHECK_MESSAGE(c.contains(QLatin1String(feld)), feld);
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == plan);
    CHECK(alsJson(kp::Bauplan{}).contains(QStringLiteral("cavity")));
    QJsonObject drei = json;
    drei.remove(QStringLiteral("cavity"));
    drei[QStringLiteral("version")] = 3;
    kp::Bauplan alt;
    REQUIRE(ausJson(drei, alt));
    CHECK_FALSE(alt.hohlraum.an());
    CHECK(alt.hohlraum == kp::Hohlraum{});

    // die Pruefung
    const auto befundMit = [](const kp::Bauplan& p, const char* stueck) {
        for (const std::string& b : kp::pruefe(p))
            if (b.find(stueck) != std::string::npos) return true;
        return false;
    };
    kp::Bauplan falsch = plan;
    falsch.hohlraum.volumen = -1.0;
    CHECK(befundMit(falsch, "cavity: volume"));
    falsch = plan;
    falsch.hohlraum.guete = 0.0;
    CHECK(befundMit(falsch, "cavity: quality"));
    falsch = plan;
    falsch.hohlraum.muendung = -0.1;
    CHECK(befundMit(falsch, "cavity: end correction"));
}

TEST_CASE("Loop Koerper 34k-5 Luftraum Oeffnungen ausserhalb der Platte (KQ17 a)")
{
    // eine Decke ohne Loch: der Hohlraum ist zu, bis er eine Oeffnung in der Schale bekommt
    kp::Bauplan plan = decke();
    plan.platte.loecher.clear();
    plan.hohlraum = {0.027, 1.7, 25.0, "Test"};
    REQUIRE(kp::pruefe(plan).empty());
    CHECK(kp::luftraumGroessen(plan, 1.2, 343.0).akustischeMasse == 0.0);

    const double flaeche = 0.0091, lippe = 0.016;
    plan.hohlraum.oeffnungen.push_back({{0.1, 0.2}, -0.15, flaeche, lippe, "Test"});
    REQUIRE(kp::pruefe(plan).empty());
    const kp::LuftraumGroessen g = kp::luftraumGroessen(plan, 1.2, 343.0);
    REQUIRE(g.loecher.size() == 1);
    const double radius = std::sqrt(flaeche / kPi);
    // Helmholtz mit einer Oeffnung: f = c/2π · √(A / (V (Lippe + Muendung · Radius)))
    CHECK(g.loecher[0].laenge == doctest::Approx(lippe + 1.7 * radius).epsilon(1e-12));
    CHECK(g.loecher[0].frei);
    CHECK(g.loecher[0].hoehe == doctest::Approx(-0.15));
    CHECK(g.loecher[0].anteil == doctest::Approx(1.0));
    CHECK(g.helmholtzHz() == doctest::Approx(343.0 / (2.0 * kPi) * std::sqrt(flaeche / (0.027 * (lippe + 1.7 * radius)))).epsilon(1e-12));

    // zwei gleiche Oeffnungen parallel: halbe Masse; ein Loch der Platte und eine Oeffnung gleicher Flaeche
    // und Laenge teilen sich den Fluss zur Haelfte
    kp::Bauplan zwei = plan;
    zwei.hohlraum.oeffnungen.push_back({{0.3, 0.2}, -0.15, flaeche, lippe, "Test"});
    const kp::LuftraumGroessen g2 = kp::luftraumGroessen(zwei, 1.2, 343.0);
    CHECK(g2.akustischeMasse == doctest::Approx(g.akustischeMasse / 2.0).epsilon(1e-12));
    CHECK(g2.helmholtzHz() == doctest::Approx(g.helmholtzHz() * std::sqrt(2.0)).epsilon(1e-12));
    kp::Bauplan gemischt = decke();
    gemischt.hohlraum = {0.012, 1.7, 25.0, "Test"};
    const double lochFlaeche = kp::flaeche(gemischt.platte.loecher[0]);
    gemischt.hohlraum.oeffnungen.push_back({{0.4, 0.1}, -0.1, lochFlaeche, 0.003, "Test"});  // Lippe = Dicke der Platte
    const kp::LuftraumGroessen g3 = kp::luftraumGroessen(gemischt, 1.2, 343.0);
    REQUIRE(g3.loecher.size() == 2);
    CHECK_FALSE(g3.loecher[0].frei);
    CHECK(g3.loecher[1].frei);
    CHECK(g3.loecher[0].anteil == doctest::Approx(0.5).epsilon(1e-9));
    CHECK(g3.loecher[1].anteil == doctest::Approx(0.5).epsilon(1e-9));

    // die Abstrahlung: ein Loch der Platte strahlt in den Halbraum (ρ/2π), eine Oeffnung frei (ρ/4π) von
    // ihrem Ort aus — an einem Hoerort gleich weit von beiden die Haelfte
    std::vector<kp::Hoerort> orte{{"vorn", {0.27, 0.15}, 0.5, "Test"}};
    kp::LuftraumGroessen nurLoch, nurOeffnung;
    nurLoch.loecher.push_back({{0.27, 0.15}, 0.01, 0.05, 1.0, 0.0, false});
    nurOeffnung.loecher.push_back({{0.27, 0.15}, 0.01, 0.05, 1.0, 0.0, true});
    const auto a = kp::lochAbstrahlung(nurLoch, orte, kp::Luft{}, 200.0, 1.0);
    const auto b = kp::lochAbstrahlung(nurOeffnung, orte, kp::Luft{}, 200.0, 1.0);
    CHECK(std::abs(a[0]) == doctest::Approx(kp::Luft{}.dichte / (2.0 * kPi) / 0.5).epsilon(1e-12));
    CHECK(std::abs(b[0]) == doctest::Approx(std::abs(a[0]) / 2.0).epsilon(1e-12));
    // die Oeffnung 0,15 m hinter der Platte ist vom Hoerort 0,5 m davor 0,65 m entfernt
    nurOeffnung.loecher[0].hoehe = -0.15;
    const auto hinten = kp::lochAbstrahlung(nurOeffnung, orte, kp::Luft{}, 200.0, 1.0);
    CHECK(std::abs(hinten[0]) == doctest::Approx(kp::Luft{}.dichte / (4.0 * kPi) / 0.65).epsilon(1e-12));

    // die Datei: `openings` immer geschrieben (seit Fassung 5), Hin- und Rueckweg; Fassung 4 ohne die Liste lesbar
    const QJsonObject json = alsJson(zwei);
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kBauplanFassung);
    const QJsonObject c = json.value(QStringLiteral("cavity")).toObject();
    REQUIRE(c.value(QStringLiteral("openings")).toArray().size() == 2);
    const QJsonObject eins = c.value(QStringLiteral("openings")).toArray()[0].toObject();
    for (const char* feld : {"x", "y", "z", "area", "lipLength", "source"}) CHECK_MESSAGE(eins.contains(QLatin1String(feld)), feld);
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == zwei);
    CHECK(alsJson(kp::Bauplan{}).value(QStringLiteral("cavity")).toObject().contains(QStringLiteral("openings")));
    QJsonObject vier = json;
    QJsonObject cVier = c;
    cVier.remove(QStringLiteral("openings"));
    vier[QStringLiteral("cavity")] = cVier;
    vier[QStringLiteral("version")] = 4;
    kp::Bauplan alt;
    REQUIRE(ausJson(vier, alt));
    CHECK(alt.hohlraum.oeffnungen.empty());
    CHECK(alt.hohlraum.volumen == doctest::Approx(0.027));
    // ein Bauplan mit Oeffnung hat einen anderen Abdruck als ohne
    CHECK(bauplanAbdruck(zwei) != bauplanAbdruck(plan));

    // die Pruefung
    const auto befundMit = [](const kp::Bauplan& p, const char* stueck) {
        for (const std::string& b : kp::pruefe(p))
            if (b.find(stueck) != std::string::npos) return true;
        return false;
    };
    kp::Bauplan falsch = plan;
    falsch.hohlraum.oeffnungen[0].flaeche = 0.0;
    CHECK(befundMit(falsch, "cavity: the area"));
    falsch = plan;
    falsch.hohlraum.oeffnungen[0].lippe = -0.01;
    CHECK(befundMit(falsch, "cavity: the lip length"));
    falsch = plan;
    falsch.hohlraum.volumen = 0.0;
    CHECK(befundMit(falsch, "cavity: openings need a volume"));
}

TEST_CASE("Loop Koerper 34k-5 Luftraum an der gerechneten Platte")
{
    kp::Bauplan plan = decke();
    const kp::LagrangeDreieck element(3);
    const Gerechnet r = rechne(plan, element, 0.05, 8);
    REQUIRE(r.moden.size() == 8);
    const kp::Luft luft;
    const kp::Auswertung auswertung(r.system, element, plan.stegpunkte, plan.hoerorte, luft, plan.daempfung);

    // das Volumen einer Form: w = 1 an jedem freien Knoten gibt (fast) die Flaeche der Platte — am eingespannten
    // Rand faellt die Form auf 0, also etwas weniger; und es ist linear
    std::vector<double> eins(static_cast<std::size_t>(r.system.k.n), 0.0);
    for (std::size_t q = 0; q < r.system.knoten.size(); ++q)
        if (const int g = r.system.frei[3 * q]; g >= 0) eins[static_cast<std::size_t>(g)] = 1.0;
    const double flaeche = kp::flaeche(plan.platte);
    CHECK(auswertung.volumen(eins) < flaeche);
    CHECK(auswertung.volumen(eins) > 0.7 * flaeche);
    std::vector<double> summe = r.moden[0].form;
    for (std::size_t i = 0; i < summe.size(); ++i) summe[i] += 2.0 * r.moden[2].form[i];
    CHECK(auswertung.volumen(summe) ==
          doctest::Approx(auswertung.volumen(r.moden[0].form) + 2.0 * auswertung.volumen(r.moden[2].form)).epsilon(1e-10));
    // die erste Mode der eingespannten Platte hat einen Bauch: sie verdraengt Volumen
    CHECK(std::abs(auswertung.volumen(r.moden[0].form)) > 1e-3);

    // ein riesiger Hohlraum: eine Mode der Luft nahe 0 Hz, die Moden der Platte bleiben
    plan.hohlraum = {1.0e9, 1.7, 30.0, "Test"};
    {
        const kp::MitLuftraum ml = kp::mitLuftraum(auswertung, plan, luft, r.moden);
        REQUIRE(ml.moden.size() == 9);
        CHECK(ml.moden[0].frequenzHz < 0.1);
        CHECK(ml.luftAnteil[0] == doctest::Approx(1.0).epsilon(1e-6));
        CHECK(ml.moden[0].verlustfaktor == doctest::Approx(1.0 / 30.0).epsilon(1e-6));
        for (std::size_t k = 0; k < r.moden.size(); ++k)
        {
            CAPTURE(k);
            const kp::ModeImKoerper allein = auswertung.werte(r.moden[k]);
            const kp::ModeImKoerper& mit = ml.moden[k + 1];
            CHECK(mit.frequenzHz == doctest::Approx(allein.frequenzHz).epsilon(1e-6));
            CHECK(std::abs(mit.amStegpunkt[0]) == doctest::Approx(std::abs(allein.amStegpunkt[0])).epsilon(1e-4));
            CHECK(std::abs(mit.abstrahlung[0]) == doctest::Approx(std::abs(allein.abstrahlung[0])).epsilon(1e-3));
            CHECK(mit.verlustfaktor == doctest::Approx(allein.verlustfaktor).epsilon(1e-4));
        }
    }

    // ein Hohlraum von 12 Litern
    plan.hohlraum = {0.012, 1.7, 30.0, "Test"};
    const kp::LuftraumGroessen g = kp::luftraumGroessen(plan, luft.dichte, luft.schall);
    const kp::MitLuftraum ml = kp::mitLuftraum(auswertung, plan, luft, r.moden);
    REQUIRE(ml.moden.size() == 9);
    MESSAGE("Helmholtz starr ", g.helmholtzHz(), " Hz; Platte allein ", r.moden[0].frequenzHz, " / ", r.moden[1].frequenzHz, " / ",
            r.moden[2].frequenzHz, " Hz; gekoppelt ", ml.moden[0].frequenzHz, " / ", ml.moden[1].frequenzHz, " / ", ml.moden[2].frequenzHz,
            " / ", ml.moden[3].frequenzHz, " Hz");
    // die unterste liegt unter Helmholtz und unter der ersten Mode der Platte, aufsteigend
    CHECK(ml.moden[0].frequenzHz < g.helmholtzHz());
    CHECK(ml.moden[0].frequenzHz < r.moden[0].frequenzHz);
    for (std::size_t k = 1; k < ml.moden.size(); ++k) CHECK(ml.moden[k].frequenzHz >= ml.moden[k - 1].frequenzHz);
    // Verschachtelung: zwischen zwei Moden der Platte liegt hoechstens eine gekoppelte mehr; keine faellt
    for (std::size_t k = 0; k < r.moden.size(); ++k) CHECK(ml.moden[k + 1].frequenzHz >= r.moden[k].frequenzHz * (1.0 - 1e-12));
    // die Spur: Σω² gekoppelt = Σω² Platte + α (Σa² + 1/M_a)
    double spur = 0.0, soll = g.alpha / g.akustischeMasse;
    for (const kp::ModeImKoerper& m : ml.moden) spur += std::pow(2.0 * kPi * m.frequenzHz, 2);
    for (const kp::Mode& m : r.moden)
        soll += std::pow(2.0 * kPi * m.frequenzHz, 2) + g.alpha * std::pow(auswertung.volumen(m.form), 2);
    CHECK(spur == doctest::Approx(soll).epsilon(1e-9));
    // der Anteil der Luft summiert sich ueber alle Moden zu 1
    double luftSumme = 0.0;
    for (const double a : ml.luftAnteil) luftSumme += a;
    CHECK(luftSumme == doctest::Approx(1.0).epsilon(1e-9));
    // jede gekoppelte Form ist wieder M-normiert bis auf den Anteil der Luft: Σ platte² = 1 − luftAnteil —
    // geprueft ueber die Mode mit dem groessten Anteil der Luft: sie strahlt auch ueber das Loch
    const std::size_t luftMode = static_cast<std::size_t>(std::max_element(ml.luftAnteil.begin(), ml.luftAnteil.end()) - ml.luftAnteil.begin());
    CHECK(luftMode == 0);
    const kp::ModeImKoerper nurPlatte = auswertung.werte(ml.formen[0]);
    CHECK(std::abs(ml.moden[0].abstrahlung[0] - nurPlatte.abstrahlung[0]) > 1e-6);
    CHECK(ml.moden[0].verlustfaktor > 0.0);
    // jede Mode traegt ihren Anteil der Luft (Koerper Fassung 3)
    for (std::size_t k = 0; k < ml.moden.size(); ++k) CHECK(ml.moden[k].luftAnteil == ml.luftAnteil[k]);
}

TEST_CASE("Loop Koerper 34k-5 Luftraum die mitgelieferte Oud (KQ10 a, KQ42 a bis KQ45 a)")
{
    const QString stamm = QStringLiteral("asset/loop/instrument_bodies/oud");
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(stamm + QLatin1String(kBauplanEndung), plan), "die Test-Exe aus dem Projekt-Root starten");
    const std::vector<std::string> befunde = kp::pruefe(plan);
    for (const std::string& b : befunde) MESSAGE(b);
    CHECK(befunde.empty());

    // Abdallah u. a. 2026: Holz Tab. 1, sieben Balken Tab. 2, drei Loecher R 5,3 / 2,0 / 2,0 cm (Abb. 3)
    const kp::Material* holz = kp::findeMaterial(plan, "sitka");
    REQUIRE(holz);
    CHECK(holz->dichte == doctest::Approx(460.0));
    CHECK(holz->ex == doctest::Approx(10.34e9));
    CHECK(plan.platte.faserWinkelGrad == doctest::Approx(90.0));
    // sieben Balken (Tab. 2) und drei Ringe, die die Masse der Rosetten tragen (KQ44 a)
    REQUIRE(plan.rippen.size() == 10);
    CHECK(plan.platte.dicke == doctest::Approx(0.00223));
    REQUIRE(plan.platte.loecher.size() == 3);
    const double soll[3] = {kPi * 0.053 * 0.053, kPi * 0.02 * 0.02, kPi * 0.02 * 0.02};
    for (std::size_t j = 0; j < 3; ++j) CHECK(std::abs(kp::flaeche(plan.platte.loecher[j])) == doctest::Approx(soll[j]).epsilon(1e-4));
    // KQ42 a: die Balken reichen bis 2 mm vor den Rand (Abb. 3 zeichnet 10–16,5 mm; so gebaut liegt der Bauch der
    // strahlenden Mode im Lochfeld statt am Steg), keiner beruehrt den Umriss
    const auto zumUmriss = [&plan](const kp::Punkt2& p) {
        double naechster = 1.0;
        const std::size_t n = plan.platte.umriss.size();
        for (std::size_t j = 0; j < n; ++j)
        {
            const kp::Punkt2& a = plan.platte.umriss[j];
            const kp::Punkt2& b = plan.platte.umriss[(j + 1) % n];
            const double dx = b.x - a.x, dy = b.y - a.y;
            const double l2 = dx * dx + dy * dy;
            const double t = l2 > 0.0 ? std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / l2, 0.0, 1.0) : 0.0;
            naechster = std::min(naechster, std::hypot(p.x - a.x - t * dx, p.y - a.y - t * dy));
        }
        return naechster;
    };
    for (std::size_t j = 0; j < 7; ++j)
    {
        const kp::Leiste& r = plan.rippen[j];
        CAPTURE(r.name);
        CHECK(r.material == "sitka");
        for (const kp::Punkt2& ende : {r.linie.front(), r.linie.back()})
        {
            if (std::abs(ende.x) < 0.08) continue;  // die inneren Enden der Balken 4 und 5 am grossen Loch
            CHECK(zumUmriss(ende) > 0.0005);
            CHECK(zumUmriss(ende) < 0.0021);
        }
    }
    // KQ44 a: die Rosetten als Masse am Lochrand — je Loch ein weicher Ring mit eigenem Werkstoff, 11,4 / 1 / 1 g,
    // fast ohne Steife (E 10 MPa gegen 10 GPa des Holzes)
    const double rosetteGramm[3] = {11.4, 1.0, 1.0};
    for (std::size_t j = 0; j < 3; ++j)
    {
        const kp::Leiste& r = plan.rippen[7 + j];
        CAPTURE(r.name);
        const kp::Material* werkstoff = kp::findeMaterial(plan, r.material);
        REQUIRE(werkstoff);
        CHECK(werkstoff->ex == doctest::Approx(1.0e7));
        CHECK_FALSE(werkstoff->quelle.empty());
        double laenge = 0.0;
        for (std::size_t q = 0; q + 1 < r.linie.size(); ++q) laenge += std::hypot(r.linie[q + 1].x - r.linie[q].x, r.linie[q + 1].y - r.linie[q].y);
        CHECK(1000.0 * werkstoff->dichte * laenge * r.breite.front() * r.hoehe.front() == doctest::Approx(rosetteGramm[j]).epsilon(0.01));
    }
    // der Steg: 17 cm lang (Abb. 3), Querschnitt und Holz angenommen und angepasst, 24,5 g
    REQUIRE(plan.stege.size() == 1);
    {
        const kp::Leiste& steg = plan.stege[0];
        const kp::Material* werkstoff = kp::findeMaterial(plan, steg.material);
        REQUIRE(werkstoff);
        const double laenge = steg.linie.back().x - steg.linie.front().x;
        CHECK(laenge == doctest::Approx(0.17));
        CHECK(steg.linie.front().y == doctest::Approx(0.09));
        CHECK(1000.0 * werkstoff->dichte * laenge * steg.breite.front() * steg.hoehe.front() == doctest::Approx(24.5).epsilon(0.01));
    }
    // der Hohlraum: die Schale 21 cm tief, 20,6 l
    REQUIRE(plan.hohlraum.an());
    CHECK(plan.hohlraum.volumen == doctest::Approx(0.0206).epsilon(0.005));
    const kp::LuftraumGroessen g = kp::luftraumGroessen(plan, kp::Luft{}.dichte, kp::Luft{}.schall);
    CHECK(plan.hohlraum.muendung == doctest::Approx(1.52));
    CHECK(g.helmholtzHz() == doctest::Approx(161.4).epsilon(0.01));
    // sechs Choere, arabische Stimmung C2 F2 A2 D3 G3 C4, am Steg 9 cm vom unteren Ende
    REQUIRE(plan.stegpunkte.size() == 6);
    const int tasten[6] = {36, 41, 45, 50, 55, 60};
    for (std::size_t s = 0; s < 6; ++s)
    {
        CHECK(plan.stegpunkte[s].taste == tasten[s]);
        CHECK(plan.stegpunkte[s].ort.y == doctest::Approx(0.09));
        CHECK_FALSE(plan.stegpunkte[s].quelle.empty());
    }
    CHECK(plan.stegpunkte[5].ort.x - plan.stegpunkte[0].ort.x == doctest::Approx(0.055));
    CHECK_FALSE(plan.daempfung.quelle.empty());

    // der Koerper: die ersten fuenf Moden des Instruments. Der Bauplan beschreibt die Oud OHNE den Aufnehmer der
    // Messung (KQ43 a); mit ihm (5,8 g am Antwortpunkt 3, als Massenaenderung im Modenraum gerechnet:
    // `…/proben/2026-11-19_S209_oud_suche/oud_kopie_pruefen.py`) liegen sie bei 139,4 / 203,4 / 274,7 / 351,2 / 389,1 Hz —
    // gegen die Spitzen der gemessenen Kurve (Abdallah u. a. 2026 Abb. 6) 139 / 203 / 280 / 362 / 385 Hz sind das
    // +0,3 / +0,2 / −1,9 / −3,0 / +1,1 %. Die Spitze bei 101 Hz hat kein Gegenstueck (KQ42 a). Hier festgehalten
    // sind die Lagen des Instruments, je auf 1 %.
    kp::Koerper k;
    REQUIRE(ladeKoerper(stamm + QLatin1String(kKoerperEndung), k));
    REQUIRE(k.moden.size() > 100);
    const double lagen[5] = {141.1, 214.1, 274.7, 372.5, 389.2};
    for (std::size_t j = 0; j < 5; ++j)
    {
        CAPTURE(j);
        CHECK(k.moden[j].frequenzHz == doctest::Approx(lagen[j]).epsilon(0.01));
    }
    // KQ42 a: die unterste ist die Mode der Luft (139 Hz gemessen), die zweite die erste Deckenmode (203 Hz); beide
    // bewegen den Steg, alle sechs Stegpunkte gleichsinnig
    CHECK(k.moden[0].luftAnteil > 0.7);
    CHECK(k.moden[0].luftAnteil < 0.9);
    CHECK(k.moden[1].luftAnteil > 0.1);
    CHECK(k.moden[1].luftAnteil < 0.25);
    for (std::size_t j = 0; j < 2; ++j)
    {
        CAPTURE(j);
        CHECK(k.moden[j].amStegpunkt[0] * k.moden[j].amStegpunkt[5] > 0.0);
        CHECK(std::abs(k.moden[j].amStegpunkt[2]) > 1.0);
    }
    CHECK(std::abs(k.moden[1].amStegpunkt[2]) > std::abs(k.moden[0].amStegpunkt[2]));
    // die gegengleiche Mode (373 Hz) hat keine Luft und bewegt die beiden Enden des Stegs gegeneinander
    CHECK(k.moden[3].luftAnteil < 0.01);
    CHECK(k.moden[3].amStegpunkt[0] * k.moden[3].amStegpunkt[5] < 0.0);

    // KQ41 a (Ziel 3 von KQ38 a): die Hauptspitze der Abstrahlung liegt beim 1,5- bis 2,0-Fachen der Luftmode
    // (Degirmenli 2017: 1,7; Degirmenli 2023: 1,5–1,6; vor S209: 2,7). Schalldruck am Hoerort je Kraft am Steg,
    // quadratisches Mittel ueber die Stegpunkte der Choere 6, 3 und 1.
    REQUIRE(k.stegTasten.size() == 6);
    REQUIRE_FALSE(k.hoerorte.empty());
    auto druckDb = [&k](double hz) {
        const double om = 2.0 * kPi * hz;
        double summe = 0.0;
        for (const std::size_t s : {std::size_t{0}, std::size_t{3}, std::size_t{5}})
        {
            std::complex<double> p = 0.0;
            for (const kp::ModeImKoerper& m : k.moden)
            {
                const double omn = 2.0 * kPi * m.frequenzHz;
                p += -om * om * m.amStegpunkt[s] * m.abstrahlung[0] / (omn * omn * std::complex<double>(1.0, m.verlustfaktor) - om * om);
            }
            summe += std::norm(p);
        }
        return 10.0 * std::log10(summe / 3.0);
    };
    double luftHz = 0.0, luftDb = -1e9, hauptHz = 0.0, hauptDb = -1e9;
    for (double hz = 120.0; hz <= 160.0; hz += 0.5)
        if (const double d = druckDb(hz); d > luftDb) luftDb = d, luftHz = hz;
    for (double hz = 170.0; hz <= 400.0; hz += 0.5)
        if (const double d = druckDb(hz); d > hauptDb) hauptDb = d, hauptHz = hz;
    CAPTURE(luftHz);
    CAPTURE(hauptHz);
    CHECK(hauptHz / luftHz > 1.5);
    CHECK(hauptHz / luftHz < 2.0);
    CHECK(hauptDb > luftDb);
    // Bekannte Abweichungen, die die Koerperdatei nicht traegt und die deshalb im Bericht stehen
    // (`…/2026-11-19_S207_spruenge_je_taste.md` §12, KQ45 a): Ziel 2 von KQ38 a (am Antwortpunkt 3 gewinnt die
    // gegengleiche Mode um 4,4 dB) und die Schwellen von KQ36 a (Spanne 19,9 LU, groesster Sprung 14,3 LU).
}

TEST_CASE("Loop Koerper 34k-5 Luftraum die mitgelieferte Laute (KQ11 a, KQ12 a)")
{
    const QString stamm = QStringLiteral("asset/loop/instrument_bodies/lute");
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(stamm + QLatin1String(kBauplanEndung), plan), "die Test-Exe aus dem Projekt-Root starten");
    const std::vector<std::string> befunde = kp::pruefe(plan);
    for (const std::string& b : befunde) MESSAGE(b);
    CHECK(befunde.empty());

    // KQ12 a: die Form der Rauwolf-Laute (475 x 326,5 mm, Rose 110 mm, Steg 139 mm), mal 625/677,4
    const double massstab = 625.0 / 677.4;
    double laenge = 0.0, breite = 0.0;
    for (const kp::Punkt2& u : plan.platte.umriss)
    {
        laenge = std::max(laenge, u.y);
        breite = std::max(breite, 2.0 * std::abs(u.x));
    }
    CHECK(laenge == doctest::Approx(0.4748 * massstab).epsilon(0.002));
    CHECK(breite == doctest::Approx(0.3265 * massstab).epsilon(0.005));
    REQUIRE(plan.platte.loecher.size() == 1);
    CHECK(std::abs(kp::flaeche(plan.platte.loecher[0])) == doctest::Approx(0.85 * kPi * std::pow(0.055 * massstab, 2)).epsilon(1e-3));
    REQUIRE(plan.stege.size() == 1);
    CHECK(plan.stege[0].linie.back().x - plan.stege[0].linie.front().x == doctest::Approx(0.139 * massstab).epsilon(1e-3));
    // sieben Querbalken, der durch die Rose geteilt; jeder reicht bis an die Muschel
    CHECK(plan.rippen.size() == 8);
    // der Hohlraum: 14,0 l der Zeichnung mal Massstab hoch drei
    REQUIRE(plan.hohlraum.an());
    CHECK(plan.hohlraum.volumen == doctest::Approx(0.014 * massstab * massstab * massstab).epsilon(1e-3));
    // acht Choere in G (Woodhouse & Lynch-Aird 2019 Tab. II): D2 F2 G2 C3 F3 A3 D4 G4
    REQUIRE(plan.stegpunkte.size() == 8);
    const int tasten[8] = {38, 41, 43, 48, 53, 57, 62, 67};
    for (std::size_t s = 0; s < 8; ++s)
    {
        CHECK(plan.stegpunkte[s].taste == tasten[s]);
        CHECK_FALSE(plan.stegpunkte[s].quelle.empty());
    }
    CHECK_FALSE(plan.daempfung.quelle.empty());

    kp::Koerper k;
    REQUIRE(ladeKoerper(stamm + QLatin1String(kKoerperEndung), k));
    REQUIRE(k.moden.size() > 100);
    // die unterste ist die Mode der Luft
    CHECK(k.moden[0].luftAnteil > 0.85);
    CHECK(k.moden[0].frequenzHz == doctest::Approx(132.0).epsilon(0.02));

    // KQ11 a: die Stegadmittanz zwischen Chor 1 und 2 (Mittel der beiden Stegpunkte), mit der Last der
    // gedaempften Saiten (2,47 N s/m), gegen die Spitzen der gemessenen Kurve (Woodhouse 2021 Abb. 1, aus
    // dem Bild gelesen): Lage innerhalb von 5 %, Hoehe innerhalb von 6 dB
    const double saiten = 2.47;
    auto admittanzDb = [&](double hz) {
        const double om = 2.0 * kPi * hz;
        std::complex<double> y = 0.0;
        for (const kp::ModeImKoerper& m : k.moden)
        {
            const double phi = 0.5 * (m.amStegpunkt[6] + m.amStegpunkt[7]);
            const double omn = 2.0 * kPi * m.frequenzHz;
            y += std::complex<double>(0.0, om) * phi * phi / (omn * omn * std::complex<double>(1.0, m.verlustfaktor) - om * om);
        }
        y /= 1.0 + saiten * y;
        return 20.0 * std::log10(std::abs(y));
    };
    const double ziele[4][2] = {{132.2, -28.0}, {277.1, -17.4}, {340.6, -19.6}, {475.6, -11.8}};
    for (const auto& ziel : ziele)
    {
        // die hoechste Stelle der gerechneten Kurve innerhalb von 8 % um das Ziel muss eine Spitze sein
        double ort = 0.0, hoehe = -1e9;
        for (double hz = ziel[0] * 0.92; hz <= ziel[0] * 1.08; hz += 0.1)
        {
            const double d = admittanzDb(hz);
            if (d > hoehe)
            {
                hoehe = d;
                ort = hz;
            }
        }
        CAPTURE(ziel[0]);
        CAPTURE(ort);
        CAPTURE(hoehe);
        CHECK(ort > ziel[0] * 0.921);
        CHECK(ort < ziel[0] * 1.079);
        CHECK(std::abs(ort / ziel[0] - 1.0) < 0.05);
        CHECK(std::abs(hoehe - ziel[1]) < 6.0);
    }
}

TEST_CASE("Loop Koerper 34k-5 Luftraum das mitgelieferte Cembalo (KQ13 a, KQ15 a, KQ16 a)")
{
    const QString stamm = QStringLiteral("asset/loop/instrument_bodies/harpsichord");
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(stamm + QLatin1String(kBauplanEndung), plan), "die Test-Exe aus dem Projekt-Root starten");
    const std::vector<std::string> befunde = kp::pruefe(plan);
    for (const std::string& b : befunde) MESSAGE(b);
    CHECK(befunde.empty());

    // KQ13 a: der Boden des Dulcken 1755 nach Bader u. a. 2025 — 166,4 x 79,7 cm (Abb. 2, 14 px je cm), Faser laengs
    double laenge = 0.0, breite = 0.0;
    for (const kp::Punkt2& u : plan.platte.umriss)
    {
        laenge = std::max(laenge, u.x);
        breite = std::max(breite, u.y);
    }
    CHECK(laenge == doctest::Approx(1.664).epsilon(0.002));
    CHECK(breite == doctest::Approx(0.797).epsilon(0.005));
    CHECK(std::abs(kp::flaeche(plan.platte.umriss)) == doctest::Approx(0.691).epsilon(0.01));
    CHECK(plan.platte.faserWinkelGrad == doctest::Approx(0.0));
    // KQ14 a: die gemessene Dicke als Raster (Abb. 1, Stufen von 0,5 mm): Bass um 5,7 mm, hinter der
    // Querleiste 4,3 mm, der Diskant duenner
    REQUIRE_FALSE(plan.platte.dickeRaster.leer());
    CHECK(plan.platte.dickeRaster.schritt == doctest::Approx(0.02));
    CHECK(plan.platte.dickeBei({0.25, 0.08}) == doctest::Approx(0.0057).epsilon(0.1));
    CHECK(plan.platte.dickeBei({1.40, 0.10}) == doctest::Approx(0.0043).epsilon(0.07));
    CHECK(plan.platte.dickeBei({1.58, 0.70}) < 0.004);
    // zwei Stege, Anhangleiste des 4', Querleiste, vier Rippen; die Rose als Loch
    CHECK(plan.stege.size() == 2);
    CHECK(plan.rippen.size() == 6);
    REQUIRE(plan.platte.loecher.size() == 1);
    CHECK(std::abs(kp::flaeche(plan.platte.loecher[0])) == doctest::Approx(kPi * 0.0385 * 0.0385).epsilon(1e-3));
    // KQ15 a: die Tiefe 20,7 cm — Helmholtz bei starrer Wand wie die Rechnung der Quelle (37 Hz), innerhalb von 3 %
    REQUIRE(plan.hohlraum.an());
    CHECK(plan.hohlraum.volumen == doctest::Approx(0.6908 * 0.207).epsilon(1e-3));
    const kp::LuftraumGroessen g = kp::luftraumGroessen(plan, kp::Luft{}.dichte, kp::Luft{}.schall);
    CHECK(std::abs(g.helmholtzHz() / 37.0 - 1.0) < 0.03);
    // die Daempfung aus der gemessenen Abklingzeit des Bodens: alpha = 6 ln 10 / 0,235 s
    CHECK(plan.daempfung.a == 0.0);
    CHECK(plan.daempfung.b == 0.0);
    CHECK(plan.daempfung.c == doctest::Approx(6.0 * std::log(10.0) / 0.235).epsilon(1e-3));
    // KQ16 a: 61 Tasten (F1 bis f''', der Umfang des Taskin); die 52 des Dulcken (35 bis 86) je mit eigenem
    // Punkt, die neun ausserhalb am Punkt der naechsten Randtaste
    REQUIRE(plan.stegpunkte.size() == 61);
    for (std::size_t s = 0; s < 61; ++s)
    {
        CHECK(plan.stegpunkte[s].taste == 29 + static_cast<int>(s));
        CHECK_FALSE(plan.stegpunkte[s].quelle.empty());
    }
    for (std::size_t s = 0; s < 6; ++s) CHECK(plan.stegpunkte[s].ort == plan.stegpunkte[6].ort);
    for (std::size_t s = 58; s < 61; ++s) CHECK(plan.stegpunkte[s].ort == plan.stegpunkte[57].ort);
    for (std::size_t s = 7; s <= 57; ++s)
    {
        // quer gleichmaessig (13,0 mm), laengs zum Diskant hin weiter vom Schwanz weg
        CHECK(plan.stegpunkte[s].ort.y - plan.stegpunkte[s - 1].ort.y == doctest::Approx(0.01304).epsilon(0.01));
        if (s > 12) CHECK(plan.stegpunkte[s].ort.x > plan.stegpunkte[s - 1].ort.x);
    }

    kp::Koerper k;
    REQUIRE(ladeKoerper(stamm + QLatin1String(kKoerperEndung), k));
    REQUIRE(k.moden.size() > 100);
    // die unterste ist die Mode der Luft; mit dem nachgiebigen Boden liegt sie unter den gemessenen 37 Hz —
    // die bekannte Abweichung nach KQ15 a (−14 %), hier festgehalten, damit sie nicht unbemerkt wandert
    CHECK(k.moden[0].luftAnteil > 0.9);
    CHECK(k.moden[0].frequenzHz == doctest::Approx(31.7).epsilon(0.02));
    CHECK(k.moden[0].frequenzHz < 37.0);
    // die erste Mode des Bodens traegt den Rest der Luft
    CHECK(k.moden[1].luftAnteil > 0.02);
    CHECK(k.moden[1].frequenzHz == doctest::Approx(96.0).epsilon(0.03));
    // jede Taste des Dulcken sieht am Steg Moden: die Summe der Quadrate ihrer Werte ist nicht null, und
    // Nachbarn unterscheiden sich
    for (std::size_t s = 6; s <= 57; ++s)
    {
        double summe = 0.0, anders = 0.0;
        for (const kp::ModeImKoerper& m : k.moden)
        {
            summe += m.amStegpunkt[s] * m.amStegpunkt[s];
            if (s > 6) anders += std::abs(m.amStegpunkt[s] - m.amStegpunkt[s - 1]);
        }
        CHECK(summe > 0.0);
        if (s > 6) CHECK(anders > 0.0);
    }
}

TEST_CASE("Loop Koerper 34k-5 Luftraum die mitgelieferte Harfe (KQ19 a, KQ21 a, KQ46 a, KQ48 a, KQ50 a bis KQ54 a)")
{
    const QString stamm = QStringLiteral("asset/loop/instrument_bodies/concert_harp");
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(stamm + QLatin1String(kBauplanEndung), plan), "die Test-Exe aus dem Projekt-Root starten");
    const std::vector<std::string> befunde = kp::pruefe(plan);
    for (const std::string& b : befunde) MESSAGE(b);
    CHECK(befunde.empty());

    // der Boden der Salvi „Orchestra“ nach Bell 1988: 131 cm, 41 -> 10 cm; der Umriss mit einer Ecke je Seite bei
    // y = 0,6 m, wo die Drehfeder der Bassseiten endet (KQ50 a, KQ51 a)
    REQUIRE(plan.platte.umriss.size() == 6);
    CHECK(plan.platte.umriss[1].x - plan.platte.umriss[0].x == doctest::Approx(0.41));
    CHECK(plan.platte.umriss[3].y == doctest::Approx(1.31));
    CHECK(plan.platte.umriss[3].x - plan.platte.umriss[4].x == doctest::Approx(0.10));
    CHECK(plan.platte.umriss[2].y == doctest::Approx(0.6));
    CHECK(plan.platte.umriss[5].y == doctest::Approx(0.6));
    CHECK(plan.platte.umriss[2].x == doctest::Approx(-plan.platte.umriss[5].x));
    // der Hohlraum mit sechs Oeffnungen in der Rueckseite der Schale (KQ17 a), als Rohr laengs (KQ59 a, KQ61 a); die
    // Muendung stellt die Helmholtz-Mode auf 190 Hz (KQ60 a, Waechter in test_LoopKoerperRohr), zwei Gueten (KQ63 a)
    REQUIRE(plan.hohlraum.an());
    REQUIRE(plan.hohlraum.alsRohr());
    CHECK(plan.hohlraum.volumen == doctest::Approx(0.027));
    CHECK(plan.hohlraum.oeffnungen.size() == 6);
    CHECK(plan.hohlraum.muendung == doctest::Approx(1.83));
    CHECK(plan.hohlraum.rohr.guete == doctest::Approx(28.0));
    CHECK(plan.platte.loecher.empty());
    // der Rand gestuetzt (KQ18 a, Feder 0); die Diskantkante mit Wegfeder (KQ20 a, KQ21 a); die beiden Bassseiten
    // gehalten mit Drehfeder 7 000 N (KQ50 a, KQ51 a)
    CHECK(plan.rand.elastisch());
    CHECK(plan.rand.drehfeder == 0.0);
    REQUIRE(plan.rand.abschnitte.size() == 3);
    CHECK(plan.rand.abschnitte[0].kanteVon == 3);
    CHECK(plan.rand.abschnitte[0].kanteBis == 3);
    CHECK(plan.rand.abschnitte[0].wegfeder == doctest::Approx(3.0e6));
    CHECK(plan.rand.abschnitte[0].drehfeder == 0.0);
    for (std::size_t a = 1; a < 3; ++a)
    {
        const kp::Randabschnitt& r = plan.rand.abschnitte[a];
        CAPTURE(r.name);
        CHECK(r.kanteVon == (a == 1 ? 1 : 5));
        CHECK(r.kanteBis == r.kanteVon);
        CHECK(r.wegfeder >= 1.0e12);
        CHECK(r.drehfeder == doctest::Approx(7000.0));
    }
    for (const kp::Randabschnitt& r : plan.rand.abschnitte) CHECK_FALSE(r.quelle.empty());
    // 46 Saiten, je eine Oese auf der Mittellinie; mit dem Pedal in der Mitte die weissen Tasten D1 ... G7.
    // KQ22 a: je chromatischer Taste 25 ... 104 ein Stegpunkt, 80 Punkte an 46 Orten — die schwarze Taste an der
    // Oese der Saite darunter (Pedal unten), die Taste 25 an der tiefsten Saite (Pedal oben)
    REQUIRE(plan.stegpunkte.size() == 80);
    const auto weiss = [](int taste) {
        const int klasse = taste % 12;
        return klasse == 0 || klasse == 2 || klasse == 4 || klasse == 5 || klasse == 7 || klasse == 9 || klasse == 11;
    };
    std::size_t saiten = 0;
    for (std::size_t s = 0; s < plan.stegpunkte.size(); ++s)
    {
        const kp::Stegpunkt& p = plan.stegpunkte[s];
        CAPTURE(p.taste);
        CHECK(p.taste == 25 + static_cast<int>(s));
        CHECK(p.ort.x == 0.0);
        CHECK_FALSE(p.quelle.empty());
        if (weiss(p.taste))
        {
            ++saiten;
            // jede Saite an ihrem eigenen Ort, zum Diskant hin
            if (s >= 2) CHECK(p.ort.y > plan.stegpunkte[s - 1].ort.y);
        }
        else if (p.taste == 25)
            CHECK(p.ort == plan.stegpunkte[s + 1].ort);
        else
            CHECK(p.ort == plan.stegpunkte[s - 1].ort);
    }
    CHECK(saiten == 46);
    CHECK_FALSE(plan.daempfung.quelle.empty());
    // KQ46 a, KQ52 a: der Koerper ist die besaitete Harfe — je Saite eine Feder an ihrer Oese, 0,3 E A / L
    // (Summe 1,80 · 10^6 N/m fuer E A / L), ohne eigenen Verlust (KQ49 a)
    REQUIRE(plan.punktfedern.size() == 46);
    double summeFedern = 0.0;
    for (const kp::Punktfeder& f : plan.punktfedern)
    {
        CAPTURE(f.ort.y);
        CHECK(f.ort.x == 0.0);
        CHECK(std::any_of(plan.stegpunkte.begin(), plan.stegpunkte.end(), [&f](const kp::Stegpunkt& p) { return p.ort == f.ort; }));
        CHECK(f.steife > 0.0);
        CHECK(f.verlustfaktor == 0.0);
        CHECK_FALSE(f.quelle.empty());
        summeFedern += f.steife;
    }
    CHECK(summeFedern == doctest::Approx(5.394e5).epsilon(1e-3));

    kp::Koerper k;
    REQUIRE(ladeKoerper(stamm + QLatin1String(kKoerperEndung), k));
    REQUIRE(k.moden.size() > 100);
    // KQ31-1 a: Deck- und Zugleiste liegen beidseits des Bodens auf derselben Linie, ihre Hoehen zaehlen zusammen
    CHECK(plan.leistenUebereinander == kp::LeistenUebereinander::Addieren);
    CHECK(plan.hohlraum.guete == doctest::Approx(4.4));

    // Die Platte eines Bauplans am Netz des Koerpers (5 kHz), bis 1,5 kHz gerechnet wie `InstrumentBodyTool` — S211:
    // dieselben Moden und Profile wie bis 5 kHz, auf 0,1 dB; mit Federn an Punkten auch deren Anteil (KQ49 a)
    const auto geber = kp::netzgeber();
    const auto loeser = kp::eigenloeser();
    const kp::Netz netz = geber.front()->vernetze(kp::netzAuftrag(plan, 5000.0));
    REQUIRE_MESSAGE(netz.fehler.empty(), netz.fehler);
    const kp::LagrangeDreieck element(3);
    const auto rechneBis1500 = [&](const kp::Bauplan& p) {
        const kp::PlattenSystem system = kp::baueSystem(netz, element, p);
        REQUIRE_MESSAGE(system.fehler.empty(), system.fehler);
        std::vector<double> werkstoffVerlust;
        for (const kp::Material& m : p.materialien) werkstoffVerlust.push_back(m.verlustfaktor);
        const kp::Verformungsenergie energie(system, element, [&](kp::Punkt2 q) { return kp::steifeBei(p, q); },
                                             static_cast<int>(p.materialien.size()));
        kp::Auswertung auswertung(system, element, p.stegpunkte, p.hoerorte, {}, p.daempfung, &energie, werkstoffVerlust, p.platte.umriss,
                                  p.rueckseite);
        if (!p.punktfedern.empty()) auswertung.setzePunktfedern(system, element, p.punktfedern);
        std::vector<kp::Mode> plattenModen;
        kp::EigenAuftrag auftrag;
        auftrag.k = &system.k;
        auftrag.m = &system.m;
        auftrag.bisHz = 1500.0;
        auftrag.jeMode = [&](const kp::Mode& mode) { plattenModen.push_back(mode); };
        const kp::EigenErgebnis erg = loeser.front()->loese(auftrag);
        REQUIRE_MESSAGE(erg.fehler.empty(), erg.fehler);
        std::sort(plattenModen.begin(), plattenModen.end(), [](const kp::Mode& a, const kp::Mode& b) { return a.frequenzHz < b.frequenzHz; });
        // die Laengsmoden des Rohrs wie im Werkzeug bis 5 kHz (KQ62 a); mit nur bis 1,5 kHz laege das Paar 0,4 % hoeher
        return kp::mitLuftraum(auswertung, p, kp::Luft{}, plattenModen, 5000.0).moden;
    };
    // KQ31-2 a, KQ40 a: die Ziele gelten ohne Saiten — die Platte ohne die Federn
    kp::Bauplan ohneSaiten = plan;
    ohneSaiten.punktfedern.clear();
    const std::vector<kp::ModeImKoerper> ohne = rechneBis1500(ohneSaiten);
    REQUIRE(ohne.size() > 15);

    const auto naechste = [](const std::vector<kp::ModeImKoerper>& moden, double hz) {
        return *std::min_element(moden.begin(), moden.end(), [hz](const kp::ModeImKoerper& a, const kp::ModeImKoerper& b) {
            return std::abs(a.frequenzHz - hz) < std::abs(b.frequenzHz - hz);
        });
    };
    // KQ60 a: das Paar 170 / 216 Hz von Kasten G in der Harfe ohne Saiten (Bell Abb. 7.8, 7.9) ist Bericht, keine
    // Schwelle; ausgegeben wird, wo die zwei Moden mit Luft ohne Saiten liegen
    {
        std::string zeile;
        for (const kp::ModeImKoerper& m : ohne)
            if (m.frequenzHz > 140.0 && m.frequenzHz < 760.0)
                zeile += " " + std::to_string(std::lround(10.0 * m.frequenzHz) / 10.0).substr(0, 5) + " (Luft " +
                         std::to_string(std::lround(100.0 * m.luftAnteil)) + " %, Q " + std::to_string(std::lround(1.0 / m.verlustfaktor)) + ")";
        MESSAGE("ohne Saiten:", zeile);
        std::string mit;
        for (const kp::ModeImKoerper& m : k.moden)
            if (m.frequenzHz > 140.0 && m.frequenzHz < 760.0)
                mit += " " + std::to_string(std::lround(10.0 * m.frequenzHz) / 10.0).substr(0, 5) + " (Luft " +
                       std::to_string(std::lround(100.0 * m.luftAnteil)) + " %, Q " + std::to_string(std::lround(1.0 / m.verlustfaktor)) + ")";
        MESSAGE("besaitet (Koerper):", mit);
    }
    // KQ60 a (1): ohne Saiten liegen die Bodenmode bei verschlossenen Oeffnungen und die Helmholtz-Mode (starre Waende,
    // alle Oeffnungen offen) auf 3 % beieinander — Bell S. 115: 191 gegen 187-190 Hz (Kasten A), 196 gegen 194 Hz (F)
    {
        kp::Bauplan zu = ohneSaiten;
        zu.hohlraum.oeffnungen.clear();
        const std::vector<kp::ModeImKoerper> moden = rechneBis1500(zu);
        const kp::ModeImKoerper* boden = nullptr;
        for (const kp::ModeImKoerper& m : moden)
            if (m.frequenzHz > 150.0 && m.luftAnteil < 0.5)
            {
                boden = &m;
                break;
            }
        REQUIRE(boden != nullptr);
        const kp::LuftraumGroessen g = kp::luftraumGroessen(plan, kp::Luft{}.dichte, kp::Luft{}.schall);
        const kp::RohrModen r = kp::rohrModen(plan.hohlraum.rohr, plan.hohlraum.volumen, kp::Luft{}.schall, 1500.0);
        const std::vector<kp::LuftraumMode> luft = kp::koppleRohr({}, {}, g, r, kp::Luft{}.dichte);
        REQUIRE_FALSE(luft.empty());
        const double helmholtz = luft.front().frequenzHz;
        MESSAGE("ohne Saiten, Oeffnungen zu: Bodenmode ", boden->frequenzHz, " Hz (Luft ", boden->luftAnteil, "); Helmholtz ", helmholtz, " Hz");
        CHECK(std::abs(boden->frequenzHz / helmholtz - 1.0) <= 0.03);
    }
    // KQ57 a: die Lagen der besaiteten Harfe gegen Bell besaitet (Abb. 7.8-7.14: S(1,1)- 187, S(1,1)+ 207, S(1,2) 257,
    // S(1,3) 334, S(1,4) 430, S(1,5) 555, S(1,6) 697 Hz) auf 7 % (KQ40 a). Die Mode im Koerper ist mit ihrer Frequenz
    // festgehalten (±1 %): mit dem Rohr liegen Laengsmoden mit Luft dazwischen. Bekannte Abweichungen mit Wert.
    struct Lage
    {
        const char* name;
        double modell, bell;
        double bekannt = 0.0;  // in %; 0 = in der Schwelle
    };
    for (const Lage& l : {Lage{"S(1,1)-", 173.0, 187.0, -7.5}, Lage{"S(1,1)+", 215.6, 207.0}, Lage{"S(1,2)", 246.8, 257.0},
                          Lage{"S(1,3)", 327.7, 334.0}, Lage{"S(1,4)", 432.1, 430.0}, Lage{"S(1,5)", 603.8, 555.0, 8.8},
                          Lage{"S(1,6)", 731.9, 697.0}})
    {
        const kp::ModeImKoerper m = naechste(k.moden, l.modell);
        const double abw = 100.0 * (m.frequenzHz / l.bell - 1.0);
        CAPTURE(l.name);
        CAPTURE(abw);
        CHECK(m.frequenzHz == doctest::Approx(l.modell).epsilon(0.01));
        if (l.bekannt == 0.0)
            CHECK(std::abs(abw) <= 7.0);
        else
        {
            CHECK(std::abs(abw) > 7.0);
            CHECK(std::abs(abw - l.bekannt) < 1.0);
        }
    }
    // KQ52 a: mit den Saiten verschieben sich die Moden wie beim Besaiten — Bell 1988 (Abb. 7.8-7.14): S(1,1)- +10,
    // S(1,2) +3, S(1,3) +5,7, S(1,4) -0,5, S(1,5) +1,8 %. Mit dem Rohr (S215) je Paar ohne -> mit Saiten festgehalten
    // (±0,3 Prozentpunkte)
    struct Verschiebung
    {
        double ohne, mit;
    };
    for (const Verschiebung& v : {Verschiebung{164.4, 173.0}, Verschiebung{231.0, 246.8}, Verschiebung{322.0, 327.7}, Verschiebung{427.2, 432.1},
                                  Verschiebung{600.4, 603.8}})
    {
        const double vorher = naechste(ohne, v.ohne).frequenzHz;
        const double nachher = naechste(k.moden, v.mit).frequenzHz;
        CAPTURE(vorher);
        CAPTURE(nachher);
        CHECK(vorher == doctest::Approx(v.ohne).epsilon(0.01));
        CHECK(std::abs(100.0 * (nachher / vorher - 1.0) - 100.0 * (v.mit / v.ohne - 1.0)) < 0.3);
    }

    // KQ48 a, KQ54 a: die Abstrahlung der besaiteten Harfe an den Saiten 33 ... 22 gegen Bell 1988 Abb. 7.23 (am
    // Seitenbild gelesen, S208): |p/F| an der Oese bei der Frequenz der leeren Saite, Kraft senkrecht zum Boden,
    // Leistungsmittel ueber 16 Hoerorte in 2,06 m wie KQ36 a (nur hier im Test, der Bauplan traegt sie nicht), gegen
    // Saite 26 bezogen; eine Saite haelt, wenn sie innerhalb von 6 dB liegt. Wie proben/2026-11-19_S209_harfe_suche.
    // Bekannte Abweichung: 6 von 12 halten (S211), rot, wenn es weniger werden.
    {
        kp::Bauplan mitOrten = plan;
        const double z0 = std::sin(10.0 * kPi / 180.0);
        for (int i = 0; i < 16; ++i)
        {
            const double z = z0 + (1.0 - z0) * (i + 0.5) / 16.0;
            const double a = i * kPi * (3.0 - std::sqrt(5.0));
            const double r = std::sqrt(1.0 - z * z);
            mitOrten.hoerorte.push_back({"k" + std::to_string(i + 1), {2.06 * r * std::cos(a), 0.655 + 2.06 * r * std::sin(a)}, 2.06 * z,
                                         "KQ36 a"});
        }
        std::vector<kp::ModeImKoerper> besaitet = rechneBis1500(mitOrten);
        REQUIRE(besaitet.size() > 15);
        // KQ60 a (2): Bell misst mit stark gedaempften Saiten (S. 147, 163) — je Saite ein Daempfer mit ihrem
        // Wellenwiderstand √(T m′) an der Oese, quer zur Saite (Anteil cos² 35°): Zuwachs des Verlustfaktors je Mode
        // 0,67 Σ Z0 φ² / ω (proben/2026-11-23_S214_harfe_saitendaempfung.py; Saiten vom Bass zum Diskant, kg/s)
        {
            const double z0[] = {5.314,  5.181,  4.773,  4.624,  4.45,   4.417,  3.978,  3.61,   3.482,  3.148,  2.998,  1.314,
                                 1.226,  1.124,  1.089,  1.043,  0.9417, 0.8947, 0.8468, 0.7965, 0.6993, 0.6455, 0.594,  0.5261,
                                 0.4966, 0.4613, 0.4291, 0.3763, 0.3274, 0.3084, 0.2678, 0.2494, 0.2291, 0.2143, 0.1932, 0.1848,
                                 0.1533, 0.1552, 0.1563, 0.1451, 0.1361, 0.1298, 0.1111, 0.1154, 0.09827, 0.09551};
            std::vector<std::size_t> oese;  // je Saite vom Bass (kleines y) zum Diskant der erste Stegpunkt an ihrer Oese
            for (std::size_t s = 0; s < plan.stegpunkte.size(); ++s)
                if (std::none_of(oese.begin(), oese.end(), [&](std::size_t o) { return plan.stegpunkte[o].ort == plan.stegpunkte[s].ort; }))
                    oese.push_back(s);
            std::sort(oese.begin(), oese.end(), [&](std::size_t a, std::size_t b) { return plan.stegpunkte[a].ort.y < plan.stegpunkte[b].ort.y; });
            REQUIRE(oese.size() == std::size(z0));
            const double anteil = std::cos(35.0 * kPi / 180.0) * std::cos(35.0 * kPi / 180.0);
            for (kp::ModeImKoerper& m : besaitet)
            {
                double summe = 0.0;
                for (std::size_t s = 0; s < oese.size(); ++s) summe += z0[s] * m.amStegpunkt[oese[s]] * m.amStegpunkt[oese[s]];
                m.verlustfaktor += anteil * summe / (2.0 * kPi * m.frequenzHz);
            }
        }
        CHECK(std::any_of(besaitet.begin(), besaitet.end(), [](const kp::ModeImKoerper& m) { return m.federAnteil > 0.1; }));
        const auto pfDb = [&](int taste, double hz) {
            const std::size_t punkt = static_cast<std::size_t>(taste - plan.stegpunkte.front().taste);
            REQUIRE(plan.stegpunkte[punkt].taste == taste);
            const double om = 2.0 * kPi * hz;
            double leistung = 0.0;
            for (std::size_t h = 1; h < mitOrten.hoerorte.size(); ++h)
            {
                std::complex<double> p = 0.0;
                for (const kp::ModeImKoerper& m : besaitet)
                {
                    const double omn = 2.0 * kPi * m.frequenzHz;
                    p += m.amStegpunkt[punkt] * (-om * om) / (omn * omn * std::complex<double>(1.0, m.verlustfaktor) - om * om) * m.abstrahlung[h];
                }
                leistung += std::norm(p) / 16.0;
            }
            return 10.0 * std::log10(leistung);
        };
        struct SaiteBell
        {
            int saite;
            double hz, db;
        };
        const SaiteBell bell[] = {{33, 104, -26}, {32, 116, -24}, {31, 123, -31}, {30, 139, -32},   {29, 155, -34}, {28, 165, -28.5},
                                  {27, 185, -10}, {26, 207, -8},  {25, 233, -16}, {24, 247, -12}, {23, 277, -5.5}, {22, 311, -8}};
        const auto tasteVon = [](double hz) { return static_cast<int>(std::lround(69.0 + 12.0 * std::log2(hz / 440.0))) + 1; };
        // KQ60 a (2): bezogen auf den Ausgleich ueber alle zwoelf Saiten — der Median der Abweichungen Modell − Bell
        std::vector<double> abw;
        for (const SaiteBell& s : bell) abw.push_back(pfDb(tasteVon(s.hz), s.hz) - s.db);
        std::vector<double> sortiert = abw;
        std::sort(sortiert.begin(), sortiert.end());
        const double median = 0.5 * (sortiert[5] + sortiert[6]);
        int halten = 0;
        double groesste = 0.0;
        std::string zeile;
        for (std::size_t i = 0; i < abw.size(); ++i)
        {
            const double a = abw[i] - median;
            halten += std::abs(a) <= 6.0 ? 1 : 0;
            groesste = std::max(groesste, std::abs(a));
            zeile += " " + std::to_string(bell[i].saite) + ":" + std::to_string(std::lround(10.0 * a) / 10.0).substr(0, 5);
        }
        MESSAGE("Abstrahlung Saite 33 ... 22 mit den Saiten als Daempfer, Modell - Bell - Median in dB:", zeile, "; halten ", halten,
                " / 12, groesste ", groesste);
        // bekannte Abweichung (S215, Rohr): 4 / 12 halten, groesste 11,4 dB — rot, wenn es weniger werden
        CHECK(halten >= 4);
        CHECK(groesste < 12.0);
    }

    // KQ31-2 a: die Harfe ohne Saiten an den Moden S(1,2) ... S(1,6), gegen Bell 1988 Abb. 7.10-7.14 Feld 2 (Kasten G
    // in der Harfe; in S206 am Seitenbild von Auge gelesen, ±1 Ort, ±2 dB; 116 dB = 1 s/kg). Je Mode die Frequenz
    // der hoechsten Spitze der Punktadmittanz ueber die Oesen (±12 %), dort das Profil laengs der Deckleiste und
    // darin die Baeuche (Maxima, die ihre Umgebung um 3 dB ueberragen). Messorte nach Abb. 6.3:
    // y = 0,035 (40 - Ort) - 0,057 m. Wie die Probe proben/2026-11-18_S206_harfe_kq31_2.py.
    std::vector<std::size_t> oesen;  // je Oese (eigener Ort) der erste Stegpunkt
    for (std::size_t s = 0; s < plan.stegpunkte.size(); ++s)
        if (oesen.empty() || plan.stegpunkte[s].ort.y > plan.stegpunkte[oesen.back()].ort.y) oesen.push_back(s);
    REQUIRE(oesen.size() == 46);
    std::reverse(oesen.begin(), oesen.end());  // vom Diskant zum Bass: der Messort steigt
    std::vector<double> ort;
    for (const std::size_t s : oesen) ort.push_back(40.0 - (plan.stegpunkte[s].ort.y + 0.057) / 0.035);
    const auto profil = [&](double hz) {
        const double om = 2.0 * kPi * hz;
        std::vector<double> v;
        for (const std::size_t punkt : oesen)
        {
            std::complex<double> y = 0.0;
            for (const kp::ModeImKoerper& m : ohne)
            {
                const double phi = m.amStegpunkt[punkt];
                const double omn = 2.0 * kPi * m.frequenzHz;
                y += std::complex<double>(0.0, om) * phi * phi / (omn * omn * std::complex<double>(1.0, m.verlustfaktor) - om * om);
            }
            v.push_back(20.0 * std::log10(std::abs(y)) + 116.0);
        }
        return v;
    };
    const auto spitzeUm = [&](double mitte, double breite) {
        double beste = 0.0, hoehe = -1e9;
        for (double hz = mitte * (1.0 - breite); hz < mitte * (1.0 + breite); hz += 0.25)
        {
            const std::vector<double> v = profil(hz);
            if (const double d = *std::max_element(v.begin(), v.end()); d > hoehe)
            {
                beste = hz;
                hoehe = d;
            }
        }
        return std::pair<double, double>{beste, hoehe};
    };
    struct Bauch
    {
        double ort, db;
    };
    const auto baeucheIn = [&](const std::vector<double>& v) {
        std::vector<Bauch> aus;
        const int n = static_cast<int>(v.size());
        for (int i = 0; i < n; ++i)
        {
            if ((i > 0 && v[i - 1] > v[i]) || (i + 1 < n && v[i + 1] > v[i])) continue;
            // je Seite der tiefste Wert, bevor es hoeher geht; laeuft die Seite bis ans Ende der Oesen, begrenzt
            // sie den Bauch nicht
            bool begrenzt = false;
            double tal = -1e9;
            for (const int schritt : {-1, 1})
            {
                double t = v[i];
                int j = i + schritt;
                while (j >= 0 && j < n && v[j] <= v[i])
                {
                    t = std::min(t, v[j]);
                    j += schritt;
                }
                if (j >= 0 && j < n)
                {
                    begrenzt = true;
                    tal = std::max(tal, t);
                }
            }
            if (begrenzt && v[i] - tal < 3.0) continue;
            Bauch b{ort[i], v[i]};
            if (i > 0 && i + 1 < n)
            {
                // Scheitel der Parabel durch die drei Oesen (sie liegen ungleich weit)
                const double x0 = ort[i - 1], x1 = ort[i], x2 = ort[i + 1];
                const double d1 = (v[i] - v[i - 1]) / (x1 - x0), d2 = (v[i + 1] - v[i]) / (x2 - x1);
                const double a = (d2 - d1) / (x2 - x0);
                if (a < 0.0)
                {
                    b.ort = 0.5 * (x0 + x1) - d1 / (2.0 * a);
                    b.db = v[i - 1] + d1 * (b.ort - x0) + a * (b.ort - x0) * (b.ort - x1);
                }
            }
            aus.push_back(b);
        }
        return aus;
    };
    // KQ31-3 a: die Schwellen bleiben; was am Koerper darueber liegt, steht hier als bekannte Abweichung mit ihrem
    // Wert — der Fall wird rot, wenn sie waechst oder wenn sie verschwindet (dann gehoert die Stelle in die Schwelle).
    // Mit der Fassung F4 (KQ51 a) liegen alle Baeuche in der Lage innerhalb von 2 Messorten; ausserhalb bleiben die
    // Hoehe des staerksten Bauchs je Mode und sechs Hoehen an S(1,5) / S(1,6) (Harfenbericht Par. 16, Par. 20).
    struct BauchZiel
    {
        double ort, db;          // Bell
        double ortBekannt = 0.0; // bekannte Abweichung der Lage in Messorten (0: in der Schwelle 2)
        double dbBekannt = 0.0;  // bekannte Abweichung der Hoehe gegen den staerksten Bauch in dB (0: in der Schwelle 6)
    };
    struct ModeZiel
    {
        const char* name;
        double hz;
        double starkBekannt;  // Hoehe des staerksten Bauchs gegen Bell — bekannte Abweichung, ±1,5 dB festgehalten
        std::vector<BauchZiel> baeuche;  // vom Diskant zum Bass, der erste ist der staerkste
        double lageBekannt = 0.0;        // bekannte Abweichung der Spitze in % (0: in der Schwelle 7 %)
        std::size_t baeucheBekannt = 0;  // bekannte Zahl der Baeuche (0: wie Bell); dann nur der staerkste geprueft
    };
    // S215 (Rohr, KQ59 a, KQ63 a): ohne Saiten tragen S(1,2)-S(1,5) 21-60 % Luft, zum groessten Teil in den
    // Oeffnungen, ihre Guete faellt auf 11-21 (Bell 31-45) — die Baeuche liegen 11-23 dB unter Bell, an S(1,3)-S(1,5)
    // fehlen Baeuche zum Bass hin (Harfenbericht §23.5)
    const std::vector<ModeZiel> ziele = {
        {"S(1,2)", 249.0, -14.5, {{13.0, 96.0}, {30.0, 78.0, 0.0, 12.9}}, -7.2},
        {"S(1,3)", 316.0, -23.4, {{7.5, 104.0}, {18.5, 89.0}, {31.5, 84.0}}, 0.0, 2},
        {"S(1,4)", 432.0, -20.3, {{6.0, 105.5}, {14.5, 91.0}, {22.5, 85.5}, {32.0, 84.0}}, 0.0, 3},
        {"S(1,5)", 545.0, -15.6, {{4.0, 101.5}, {10.5, 93.0}, {16.5, 82.0}, {25.0, 86.0}, {33.5, 86.5}}, 0.0, 3},
        {"S(1,6)", 709.0, -11.5,
         {{3.5, 101.0}, {8.5, 95.5, 0.0, -6.0}, {14.0, 90.0, 0.0, -8.2}, {20.0, 86.5, 0.0, -10.9}, {27.5, 89.5, 0.0, -18.5}, {34.0, 88.5, 0.0, -20.6}}},
    };
    for (const ModeZiel& z : ziele)
    {
        const double hz = spitzeUm(z.hz, 0.12).first;
        const std::vector<Bauch> ist = baeucheIn(profil(hz));
        CAPTURE(z.name);
        CAPTURE(hz);
        {
            std::string zeile;
            for (const Bauch& b : ist)
                zeile += " Ort " + std::to_string(std::lround(10.0 * b.ort) / 10.0).substr(0, 4) + " " +
                         std::to_string(std::lround(10.0 * b.db) / 10.0).substr(0, 5) + " dB;";
            MESSAGE(z.name, " ohne Saiten bei ", hz, " Hz:", zeile);
        }
        // eine Spitze, nicht der Rand des Fensters; Lage innerhalb 7 % (oder die bekannte Abweichung)
        const double lageProzent = 100.0 * (hz / z.hz - 1.0);
        if (z.lageBekannt == 0.0)
            CHECK(std::abs(lageProzent) <= 7.0);
        else
        {
            CHECK(std::abs(lageProzent) > 7.0);
            CHECK(std::abs(lageProzent - z.lageBekannt) < 1.0);
        }
        REQUIRE_FALSE(ist.empty());
        // der staerkste Bauch liegt am Diskantende wie bei Bell; seine Hoehe ist bekannte Abweichung
        for (const Bauch& b : ist) CHECK(b.db <= ist.front().db);
        CHECK(std::abs(ist.front().db - z.baeuche.front().db - z.starkBekannt) < 1.5);
        // die Zahl der Baeuche (oder die bekannte Abweichung — dann nur der staerkste)
        if (z.baeucheBekannt != 0)
        {
            CHECK(ist.size() == z.baeucheBekannt);
            CHECK(ist.size() != z.baeuche.size());
            continue;
        }
        CHECK(ist.size() == z.baeuche.size());
        if (ist.size() != z.baeuche.size()) continue;
        for (std::size_t b = 0; b < ist.size(); ++b)
        {
            const BauchZiel& soll = z.baeuche[b];
            const double lage = ist[b].ort - soll.ort;
            const double hoehe = (ist[b].db - ist.front().db) - (soll.db - z.baeuche.front().db);
            CAPTURE(b);
            CAPTURE(lage);
            CAPTURE(hoehe);
            if (soll.ortBekannt == 0.0)
                CHECK(std::abs(lage) <= 2.0);
            else
            {
                CHECK(std::abs(lage) > 2.0);
                CHECK(std::abs(lage - soll.ortBekannt) < 0.3);
            }
            if (soll.dbBekannt == 0.0)
                CHECK(std::abs(hoehe) <= 6.0);
            else
            {
                CHECK(std::abs(hoehe) > 6.0);
                CHECK(std::abs(hoehe - soll.dbBekannt) < 1.5);
            }
        }
    }
    // KQ60 a (3): das Profil von S(1,1)- ohne Saiten (Bell Abb. 7.8, 2; Harfenbericht §22.6): ein Bauch um y = 0,45 m
    // (Ort 25,5) auf zwei Messorte, ein Knoten (tiefstes Minimum) an Ort 14-16. Das Modell hat einen langen Bauch weiter
    // zum Diskant und keinen Knoten — beides bekannte Abweichung mit Wert; die Suche nach der Form steht aus (KQ59 a)
    {
        const double hz = spitzeUm(164.4, 0.05).first;
        const std::vector<double> v = profil(hz);
        const std::size_t oben = static_cast<std::size_t>(std::max_element(v.begin(), v.end()) - v.begin());
        double tiefst = 1e9, tiefstOrt = 0.0;
        for (std::size_t i = 1; i + 1 < v.size(); ++i)
            if (v[i] < v[i - 1] && v[i] < v[i + 1] && v[i] < tiefst)
            {
                tiefst = v[i];
                tiefstOrt = ort[i];
            }
        MESSAGE("S(1,1)- ohne Saiten bei ", hz, " Hz: Bauch an Ort ", ort[oben], " (Bell 25,5), tiefstes Minimum ",
                (tiefst < 1e9 ? std::to_string(tiefstOrt) : std::string("keines")), " (Bell Ort 14-16), ",
                (tiefst < 1e9 ? v[oben] - tiefst : 0.0), " dB unter dem Bauch");
        // der Bauch haelt mit dem Rohr (Ort 23,9); der Knoten fehlt — bekannte Abweichung, rot, wenn er erscheint
        CHECK(std::abs(ort[oben] - 25.5) <= 2.0);
        CHECK_FALSE((tiefst < 1e9 && tiefstOrt >= 14.0 && tiefstOrt <= 16.0 && v[oben] - tiefst > 6.0));
    }
    // bekannte Abweichung: S(1,8) (905 Hz, 102 dB an Ort 4) liegt ohne Saiten 17 dB zu tief — das Diskantende ist
    // auch mit der Feder an der Kante zu steif (Bericht Par. 12)
    const std::pair<double, double> acht = spitzeUm(905.0, 0.08);
    CAPTURE(acht.first);
    CAPTURE(acht.second);
    CHECK(acht.second - 102.0 < -6.0);
    CHECK(acht.second - 102.0 > -20.0);
}
