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
 * - Die mitgelieferte Oud (KQ10 a; vom Arbeitsordner aus): der Bauplan mit drei Loechern, Hohlraum und
 *   sechs Stegpunkten, der Koerper mit seinen Moden an den Spitzen der gemessenen Kurve (Abdallah u. a.
 *   2026, Abb. 6: 101 / 203 / 280 / 362 Hz innerhalb von 3 %), die unterste ist die Mode der Luft.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Luftraum.hpp"
#include "InstrumentBody/NetzCdt.hpp"
#include "InstrumentBody/Platte.hpp"
#include "loop/Koerper.hpp"
#include "loop/KoerperBauplan.hpp"

#include <Eigen/Dense>

#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>
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
    g.system = kp::baueSystem(netz, element, [&](kp::Punkt2 p) { return kp::steifeBei(plan, p); }, kp::Auflager::Eingespannt);
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

TEST_CASE("Loop Koerper 34k-5 Luftraum die mitgelieferte Oud (KQ10 a)")
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
    CHECK(plan.rippen.size() == 7);
    CHECK(plan.stege.empty());
    REQUIRE(plan.platte.loecher.size() == 3);
    const double soll[3] = {kPi * 0.053 * 0.053, kPi * 0.02 * 0.02, kPi * 0.02 * 0.02};
    for (std::size_t j = 0; j < 3; ++j) CHECK(std::abs(kp::flaeche(plan.platte.loecher[j])) == doctest::Approx(soll[j]).epsilon(1e-4));
    // KQ10 a: die Balken enden vor dem Rand (wie Abb. 3 sie zeichnet), keiner beruehrt den Umriss
    for (const kp::Leiste& r : plan.rippen)
        for (const kp::Punkt2& ende : {r.linie.front(), r.linie.back()})
        {
            if (std::abs(ende.x) < 0.08) continue;  // die inneren Enden der Balken 4 und 5 am grossen Loch
            double naechster = 1.0;
            for (const kp::Punkt2& u : plan.platte.umriss) naechster = std::min(naechster, std::hypot(u.x - ende.x, u.y - ende.y));
            CHECK(naechster > 0.008);
            CHECK(naechster < 0.03);
        }
    // der Hohlraum: die Schale 21 cm tief, 20,6 l
    REQUIRE(plan.hohlraum.an());
    CHECK(plan.hohlraum.volumen == doctest::Approx(0.0206).epsilon(0.005));
    const kp::LuftraumGroessen g = kp::luftraumGroessen(plan, kp::Luft{}.dichte, kp::Luft{}.schall);
    CHECK(g.helmholtzHz() == doctest::Approx(154.0).epsilon(0.01));
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

    // der Koerper: Moden an den Spitzen der gemessenen Kurve (Abb. 6), jede innerhalb von 3 %
    kp::Koerper k;
    REQUIRE(ladeKoerper(stamm + QLatin1String(kKoerperEndung), k));
    REQUIRE(k.moden.size() > 100);
    for (const double spitze : {101.0, 203.0, 280.0, 362.0})
    {
        double naechste = 0.0;
        for (const kp::ModeImKoerper& m : k.moden)
            if (std::abs(m.frequenzHz - spitze) < std::abs(naechste - spitze)) naechste = m.frequenzHz;
        CAPTURE(spitze);
        CAPTURE(naechste);
        CHECK(std::abs(naechste / spitze - 1.0) < 0.03);
    }
    // die unterste ist die Mode der Luft, die erste Deckenmode traegt den Rest
    CHECK(k.moden[0].luftAnteil > 0.6);
    CHECK(k.moden[0].luftAnteil < 0.7);
    CHECK(k.moden[2].luftAnteil > 0.25);
    // die gegengleiche Mode dazwischen (169 Hz) hat kein Volumen und keine Luft
    CHECK(k.moden[1].luftAnteil < 0.01);
    CHECK(k.moden[1].frequenzHz == doctest::Approx(169.0).epsilon(0.02));
}
