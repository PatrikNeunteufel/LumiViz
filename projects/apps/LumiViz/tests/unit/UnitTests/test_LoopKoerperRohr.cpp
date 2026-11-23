/**
 ****************************************************************************************
 * @file   test_LoopKoerperRohr.cpp
 * @brief  Stufe 34k-5: die Luft des Hohlraums als Rohr laengs y (Entscheide Patrik S214 / S215: KQ59 a, KQ61 a bis
 *         KQ65 a; Harfenbericht §22–§23)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung (der Bauplan wird vom Arbeitsordner aus gelesen).
 *
 * - Die Rohrmoden eines Rohrs gleichen Querschnitts gegen n c / (2 L), die Normierung Σ S ψ² Δy = V.
 * - Das Rohr ohne Laengsmoden ist der eine Druck: dieselben Moden wie `koppleLuftraum`; zwei Oeffnungen geben eine
 *   Mode der Frequenz 0 (Durchfluss ohne Druck), die wegfaellt.
 * - Der Bauplan: `cavity.kind` und `cavity.tube` immer geschrieben, Hin- und Rueckweg, Fassung 9 liest sich als ein
 *   Druck, eine unbekannte Art macht die Datei unlesbar; die Pruefung (Querschnitte, Volumen, Oeffnungen im Rohr).
 * - KQ71 a (S218): die Oeffnungen aus ihrer Geometrie — die Flaechenintegrale, die Massenmatrix (Wand, frei), die
 *   Guete eines Helmholtz-Resonators aus der Abstrahlung (ω M / R), der innere Verlust aus einer gemessenen Guete;
 *   Bauplan-Fassung 11 (`openingModel`, `surroundings`, `innerLoss`, je Oeffnung `length`) im Hin- und Rueckweg.
 * - KQ71 a (loest KQ64 a ab): die Luft der mitgelieferten Harfe bei starren Waenden gegen Bell 1988 Kap. 4 (Kasten A im
 *   Sand; Abb. 4.8, 4.11–4.15, am Seitenbild gelesen S215 / S216): alle 36 Lagen (sechs Zustaende) auf 3 %, rms unter
 *   6,5 %; die zwoelf Gueten auf den Faktor 1,5; die Helmholtz-Mode mit allen Oeffnungen bei 190 ± 3 Hz ohne angepassten
 *   Wert und ihr Abfall zum Diskant (Bell rund 18 dB von 10 bis 120 cm, auf 3 dB). Was darueber liegt, steht als
 *   bekannte Abweichung mit Wert — rot, wenn sie waechst oder verschwindet (Ursache offen, Harfenbericht §24.2).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Luftraum.hpp"
#include "loop/KoerperBauplan.hpp"

#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>
#include <vector>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

constexpr double kPi = std::numbers::pi;

/// Ein Loch fuer `LuftraumGroessen` von Hand: Flaeche, wirksame Laenge, Ort laengs.
kp::LochLuft loch(double y, double flaeche, double laenge)
{
    kp::LochLuft l;
    l.mitte = {0.0, y};
    l.flaeche = flaeche;
    l.laenge = laenge;
    l.anteil = 1.0;
    l.frei = true;
    return l;
}

/// Ein gerades Rohr: Querschnitt 0,01 m², Laenge 1 m, Volumen 0,01 m³.
kp::Rohr gerade()
{
    kp::Rohr r;
    r.querschnitte = {{0.0, 0.01}, {1.0, 0.01}};
    r.guete = 30.0;
    return r;
}

/// Der Druck laengs der Mode an der Stelle y.
double druckBei(const kp::LuftraumMode& m, const kp::RohrModen& r, double y)
{
    const auto z = static_cast<std::size_t>(r.abschnittBei(y));
    double p = 0.0;
    for (std::size_t j = 0; j < m.druck.size(); ++j) p += m.druck[j] * r.psi[j][z];
    return p;
}

/// Die Guete einer Luftmode bei starren Waenden (KQ63 a; KQ71 a: Abstrahlung und innerer Verlust).
double gueteVon(const kp::LuftraumMode& m)
{
    return 1.0 / m.luftVerlust;
}

}  // namespace

TEST_CASE("Loop Koerper KQ59 Rohr gleichen Querschnitts gegen n c / 2 L")
{
    const kp::RohrModen r = kp::rohrModen(gerade(), 0.01, 340.0, 2000.0);
    REQUIRE(r.vollstaendig);
    REQUIRE(r.abschnitte >= kp::kRohrAbschnitteMin);
    CHECK(r.omega[0] == 0.0);
    // bis 2 kHz: 170, 340, ..., 1870 Hz — elf Moden ueber der nullten
    REQUIRE(r.omega.size() == 12);
    for (std::size_t j = 1; j < r.omega.size(); ++j)
    {
        CAPTURE(j);
        // finite Volumen mit 20 Abschnitten je Wellenlaenge an der Grenze: dort rund 0,4 % zu tief
        CHECK(r.hz(j) == doctest::Approx(170.0 * static_cast<double>(j)).epsilon(0.005));
        CHECK(r.hz(j) <= 170.0 * static_cast<double>(j));
        double norm = 0.0;
        for (int z = 0; z < r.abschnitte; ++z)
            norm += r.flaeche[static_cast<std::size_t>(z)] * r.schritt * r.psi[j][static_cast<std::size_t>(z)] * r.psi[j][static_cast<std::size_t>(z)];
        CHECK(norm == doctest::Approx(0.01).epsilon(1e-9));
        CHECK(r.psi[j][0] > 0.0);
        // cos(j π y / L): j Nullstellen laengs
        int wechsel = 0;
        for (int z = 1; z < r.abschnitte; ++z)
            wechsel += (r.psi[j][static_cast<std::size_t>(z)] > 0.0) != (r.psi[j][static_cast<std::size_t>(z - 1)] > 0.0) ? 1 : 0;
        CHECK(wechsel == static_cast<int>(j));
    }
    for (const double v : r.psi[0]) CHECK(v == 1.0);
}

TEST_CASE("Loop Koerper KQ59 Rohr ohne Laengsmoden wie ein Druck")
{
    // ein Loch, zwei Plattenmoden mit Volumen: das Rohr nur mit j = 0 ist der eine Druck aus `koppleLuftraum`
    const double dichte = 1.2, schall = 343.0, volumen = 0.01;
    kp::LuftraumGroessen g;
    g.alpha = dichte * schall * schall / volumen;
    g.loecher = {loch(0.4, 0.005, 0.03)};
    g.akustischeMasse = dichte * 0.03 / 0.005;
    kp::RohrModen r = kp::rohrModen(gerade(), volumen, schall, 2000.0);
    r.omega.resize(1);
    r.psi.resize(1);
    const std::vector<double> hz = {150.0, 320.0};
    std::vector<std::vector<double>> laengs(2, std::vector<double>(static_cast<std::size_t>(r.abschnitte), 0.0));
    laengs[0][3] = 0.004;   // das Volumen irgendwo laengs — mit ψ_0 = 1 zaehlt nur die Summe
    laengs[0][40] = 0.002;
    laengs[1][10] = -0.003;
    const auto rohr = kp::koppleRohr(hz, laengs, g, r, dichte);
    const auto eins = kp::koppleLuftraum(hz, {0.006, -0.003}, g);
    REQUIRE(rohr.size() == eins.size());
    for (std::size_t m = 0; m < rohr.size(); ++m)
    {
        CAPTURE(m);
        CHECK(rohr[m].frequenzHz == doctest::Approx(eins[m].frequenzHz).epsilon(1e-9));
        CHECK(rohr[m].luftAnteil == doctest::Approx(eins[m].luftAnteil).epsilon(1e-9));
        CHECK(rohr[m].rohrAnteil == 0.0);
        CHECK(std::abs(rohr[m].oeffnung) == doctest::Approx(std::abs(eins[m].oeffnung)).epsilon(1e-9));
    }

    // zwei Oeffnungen am einen Druck: Luft, die durch die eine hinein und die andere hinaus stroemt, aendert den
    // Druck nicht (Frequenz 0) — sie faellt weg; die uebrigen wie mit beiden zusammengelegt
    kp::LuftraumGroessen zwei = g;
    zwei.loecher = {loch(0.2, 0.005, 0.03), loch(0.8, 0.005, 0.03)};
    zwei.akustischeMasse = g.akustischeMasse / 2.0;
    const auto rohr2 = kp::koppleRohr(hz, laengs, zwei, r, dichte);
    const auto eins2 = kp::koppleLuftraum(hz, {0.006, -0.003}, zwei);
    REQUIRE(rohr2.size() == eins2.size());
    for (std::size_t m = 0; m < rohr2.size(); ++m) CHECK(rohr2[m].frequenzHz == doctest::Approx(eins2[m].frequenzHz).epsilon(1e-9));
}

TEST_CASE("Loop Koerper KQ59 Rohr im Bauplan und in der Datei")
{
    kp::Bauplan plan;
    plan.titel = "Kasten";
    plan.platte.umriss = {{-0.1, 0.0}, {0.1, 0.0}, {0.1, 1.0}, {-0.1, 1.0}};
    plan.hohlraum.volumen = 0.01;
    plan.hohlraum.oeffnungen = {{{0.0, 0.5}, -0.05, 0.004, 0.01, "Test"}};
    plan.hohlraum.art = kp::HohlraumArt::Rohr;
    plan.hohlraum.rohr = gerade();
    plan.hohlraum.rohr.quelle = "Test";
    CHECK(plan.hohlraum.alsRohr());
    CHECK(plan.hohlraum.rohr.volumen() == doctest::Approx(0.01));
    CHECK(plan.hohlraum.rohr.flaecheBei(0.5) == doctest::Approx(0.01));

    const auto befundMit = [](const kp::Bauplan& p, const char* stueck) {
        for (const std::string& b : kp::pruefe(p))
            if (b.find(stueck) != std::string::npos) return true;
        return false;
    };
    CHECK_FALSE(befundMit(plan, "cavity"));
    kp::Bauplan falsch = plan;
    falsch.hohlraum.rohr.querschnitte[1].flaeche = 0.012;  // 11 % mehr Volumen
    CHECK(befundMit(falsch, "cavity tube: the sections must hold the volume"));
    falsch = plan;
    falsch.hohlraum.rohr.querschnitte = {{0.0, 0.01}};
    CHECK(befundMit(falsch, "cavity tube: needs two sections"));
    falsch = plan;
    falsch.hohlraum.rohr.querschnitte[1].y = 0.0;
    CHECK(befundMit(falsch, "cavity tube: sections need ascending y"));
    falsch = plan;
    falsch.hohlraum.rohr.guete = 0.0;
    CHECK(befundMit(falsch, "cavity tube: quality"));
    falsch = plan;
    falsch.hohlraum.oeffnungen[0].ort.y = 1.2;
    CHECK(befundMit(falsch, "cavity tube: an opening lies outside"));

    // die Datei: `kind` und `tube` immer geschrieben, Hin- und Rueckweg
    const QJsonObject json = alsJson(plan);
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kBauplanFassung);
    const QJsonObject c = json.value(QStringLiteral("cavity")).toObject();
    CHECK(c.value(QStringLiteral("kind")).toString() == QStringLiteral("tube"));
    CHECK(c.value(QStringLiteral("tube")).toObject().contains(QStringLiteral("sections")));
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == plan);
    const QJsonObject ohne = alsJson(kp::Bauplan{}).value(QStringLiteral("cavity")).toObject();
    CHECK(ohne.value(QStringLiteral("kind")).toString() == QStringLiteral("singlePressure"));
    CHECK(ohne.contains(QStringLiteral("tube")));
    // Fassung 9 ohne die Felder: ein Druck
    QJsonObject neun = json;
    QJsonObject c9 = c;
    c9.remove(QStringLiteral("kind"));
    c9.remove(QStringLiteral("tube"));
    neun[QStringLiteral("cavity")] = c9;
    neun[QStringLiteral("version")] = 9;
    kp::Bauplan alt;
    REQUIRE(ausJson(neun, alt));
    CHECK(alt.hohlraum.art == kp::HohlraumArt::EinDruck);
    CHECK(alt.hohlraum.rohr == kp::Rohr{});
    // eine unbekannte Art: nicht raten
    QJsonObject fremd = json;
    QJsonObject cf = c;
    cf[QStringLiteral("kind")] = QStringLiteral("horn");
    fremd[QStringLiteral("cavity")] = cf;
    kp::Bauplan nichts;
    CHECK_FALSE(ausJson(fremd, nichts));
}

TEST_CASE("Loop Koerper KQ71 die Oeffnungen aus ihrer Geometrie: Integrale, Massen, Abstrahlung")
{
    // die Zelle mit sich selbst: ∬∬ dS dS′ / r ueber das Einheitsquadrat, geschlossen
    CHECK(kp::kEigenQuadrat == doctest::Approx(2.9732095).epsilon(1e-7));
    const auto oeffnung = [](double y, double laenge, double breite, double lippe) {
        kp::Oeffnung o;
        o.ort = {0.0, y};
        o.flaeche = laenge * breite;
        o.laenge = laenge;
        o.lippe = lippe;
        return o;
    };
    // ein Quadrat 10 cm: das Integral ueber die Zellen gegen 2,9732 a³ (die Zellen naehern die Nachbarn)
    {
        const std::vector<double> g = kp::oeffnungsIntegrale({oeffnung(0.5, 0.10, 0.10, 0.0)});
        CHECK(g[0] == doctest::Approx(kp::kEigenQuadrat * 1e-3).epsilon(0.005));
    }
    // zwei kleine Quadrate weit auseinander: G_12 → A1 A2 / d
    {
        const std::vector<double> g = kp::oeffnungsIntegrale({oeffnung(0.0, 0.01, 0.01, 0.0), oeffnung(1.0, 0.02, 0.01, 0.0)});
        CHECK(g[1] == doctest::Approx(1e-4 * 2e-4 / 1.0).epsilon(1e-4));
        CHECK(g[1] == g[2]);
    }
    // die Massen: eigene Korrektur innen und aussen, Nachbarn in der Wand ganz, frei halb und aussen 0,72
    {
        const double rho = 1.2;
        const std::vector<kp::Oeffnung> oe = {oeffnung(0.2, 0.16, 0.05, 0.015), oeffnung(0.42, 0.18, 0.05, 0.012)};
        const std::vector<double> g = kp::oeffnungsIntegrale(oe);
        const std::vector<double> wand = kp::oeffnungsMassen(oe, kp::OeffnungsUmgebung::Wand, rho);
        const std::vector<double> frei = kp::oeffnungsMassen(oe, kp::OeffnungsUmgebung::Frei, rho);
        for (std::size_t i = 0; i < 2; ++i)
        {
            const double eigen = rho * g[i * 2 + i] / (2.0 * kPi * oe[i].flaeche * oe[i].flaeche);
            CHECK(wand[i * 2 + i] == doctest::Approx(rho * oe[i].lippe / oe[i].flaeche + 2.0 * eigen));
            CHECK(frei[i * 2 + i] == doctest::Approx(rho * oe[i].lippe / oe[i].flaeche + 1.72 * eigen));
            // die eigene Korrektur aussen eines Rechtecks 16 x 5 cm in Radien des flaechengleichen Kreises: unter
            // 0,85 (Kreis), der Harfenbericht §24.2 nennt 0,71–0,80 fuer die Oeffnungen der Harfe
            const double r = std::sqrt(oe[i].flaeche / kPi);
            CHECK(eigen * oe[i].flaeche / rho / r > 0.6);
            CHECK(eigen * oe[i].flaeche / rho / r < 0.85);
        }
        CHECK(wand[1] == doctest::Approx(rho * g[1] / (2.0 * kPi * oe[0].flaeche * oe[1].flaeche)));
        CHECK(frei[1] == doctest::Approx(0.5 * wand[1]));
        CHECK(wand[1] == wand[2]);
    }

    // ein Helmholtz-Resonator mit einer Oeffnung (nur j = 0): die Guete aus der Abstrahlung ist ω M / R mit
    // R = ρ c k² / 2π (Wand), frei das Doppelte; ohne inneren Verlust
    {
        const kp::Luft luft;
        kp::Bauplan plan;
        plan.platte.umriss = {{-0.1, 0.0}, {0.1, 0.0}, {0.1, 1.0}, {-0.1, 1.0}};
        plan.hohlraum.volumen = 0.01;
        plan.hohlraum.art = kp::HohlraumArt::Rohr;
        plan.hohlraum.rohr = gerade();
        plan.hohlraum.oeffnungen = {oeffnung(0.5, 0.08, 0.05, 0.01)};
        plan.hohlraum.modell = kp::OeffnungsModell::Geometrie;
        plan.hohlraum.innen = {{0}, {{100.0, 50.0}}, "Test"};
        kp::RohrModen r = kp::rohrModen(plan.hohlraum.rohr, plan.hohlraum.volumen, luft.schall, 2000.0);
        r.omega.resize(1);
        r.psi.resize(1);
        for (const kp::OeffnungsUmgebung u : {kp::OeffnungsUmgebung::Wand, kp::OeffnungsUmgebung::Frei})
        {
            plan.hohlraum.umgebung = u;
            kp::LuftraumGroessen g = kp::luftraumGroessen(plan, luft.dichte, luft.schall);
            REQUIRE(g.geometrie);
            REQUIRE(g.massen.size() == 1);
            CHECK(g.loecher[0].feld == doctest::Approx(u == kp::OeffnungsUmgebung::Wand ? std::sqrt(2.0) : 1.0));
            const auto moden = kp::koppleRohr({}, {}, g, r, luft.dichte);
            REQUIRE(moden.size() == 1);
            const double f = moden[0].frequenzHz;
            CHECK(f == doctest::Approx(std::sqrt(g.alpha / g.massen[0]) / (2.0 * kPi)).epsilon(1e-9));
            const double k = 2.0 * kPi * f / luft.schall;
            const double widerstand = (u == kp::OeffnungsUmgebung::Wand ? 1.0 : 0.5) * luft.dichte * luft.schall * k * k / (2.0 * kPi);
            CHECK(moden[0].luftAnteil == doctest::Approx(1.0));
            CHECK(1.0 / moden[0].luftVerlust == doctest::Approx(2.0 * kPi * f * g.massen[0] / widerstand).epsilon(1e-9));
            // der innere Verlust: die gemessene Guete 50 bei 100 Hz minus die Abstrahlung an der Mode
            const kp::LuftraumGroessen mitInnen = kp::rohrGroessen(plan, luft, r);
            REQUIRE(mitInnen.innenHz.size() == 1);
            CHECK(mitInnen.innenKehrwert[0] == doctest::Approx(std::max(0.0, 1.0 / 50.0 - moden[0].luftVerlust)));
            CHECK(mitInnen.innenBei(30.0) == mitInnen.innenKehrwert[0]);
            CHECK(mitInnen.innenBei(3000.0) == mitInnen.innenKehrwert[0]);
            const auto mitVerlust = kp::koppleRohr({}, {}, mitInnen, r, luft.dichte);
            CHECK(mitVerlust[0].luftVerlust == doctest::Approx(moden[0].luftVerlust + mitInnen.innenKehrwert[0]));
        }
    }
    // die Tafel des inneren Verlusts: linear ueber f, an den Enden gehalten
    {
        kp::LuftraumGroessen g;
        g.innenHz = {100.0, 300.0};
        g.innenKehrwert = {0.1, 0.02};
        CHECK(g.innenBei(50.0) == 0.1);
        CHECK(g.innenBei(200.0) == doctest::Approx(0.06));
        CHECK(g.innenBei(400.0) == 0.02);
        CHECK(kp::LuftraumGroessen{}.innenBei(200.0) == 0.0);
    }
}

TEST_CASE("Loop Koerper KQ71 die Oeffnungen aus ihrer Geometrie im Bauplan und in der Datei")
{
    kp::Bauplan plan;
    plan.titel = "Kasten";
    plan.platte.umriss = {{-0.1, 0.0}, {0.1, 0.0}, {0.1, 1.0}, {-0.1, 1.0}};
    plan.hohlraum.volumen = 0.01;
    plan.hohlraum.oeffnungen = {{{0.0, 0.3}, -0.05, 0.004, 0.01, "Test", 0.08}, {{0.0, 0.7}, -0.05, 0.003, 0.01, "Test", 0.06}};
    plan.hohlraum.art = kp::HohlraumArt::Rohr;
    plan.hohlraum.rohr = gerade();
    plan.hohlraum.modell = kp::OeffnungsModell::Geometrie;
    plan.hohlraum.umgebung = kp::OeffnungsUmgebung::Frei;
    plan.hohlraum.innen = {{1}, {{80.0, 10.0}, {300.0, 20.0}}, "Test"};
    CHECK(plan.hohlraum.ausGeometrie());

    const auto befundMit = [](const kp::Bauplan& p, const char* stueck) {
        for (const std::string& b : kp::pruefe(p))
            if (b.find(stueck) != std::string::npos) return true;
        return false;
    };
    CHECK_FALSE(befundMit(plan, "cavity"));
    kp::Bauplan falsch = plan;
    falsch.hohlraum.art = kp::HohlraumArt::EinDruck;
    CHECK(befundMit(falsch, "the opening model 'geometry' needs a tube"));
    falsch = plan;
    falsch.platte.loecher = {{{-0.02, 0.4}, {0.02, 0.4}, {0.02, 0.44}, {-0.02, 0.44}}};
    CHECK(befundMit(falsch, "takes no holes in the plate"));
    falsch = plan;
    falsch.hohlraum.oeffnungen[1].laenge = 0.0;
    CHECK(befundMit(falsch, "needs the length of each opening"));
    falsch = plan;
    falsch.hohlraum.oeffnungen[1].laenge = -0.1;
    CHECK(befundMit(falsch, "the length of an opening must not be negative"));
    falsch = plan;
    falsch.hohlraum.innen.offen = {2};
    CHECK(befundMit(falsch, "an open opening does not exist"));
    falsch = plan;
    falsch.hohlraum.innen.punkte = {};
    CHECK(befundMit(falsch, "needs open openings and measured points"));
    falsch = plan;
    falsch.hohlraum.innen.punkte[1].hz = 50.0;
    CHECK(befundMit(falsch, "points need ascending frequency"));

    // die Datei: alles immer geschrieben, Hin- und Rueckweg
    const QJsonObject json = alsJson(plan);
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kBauplanFassung);
    const QJsonObject c = json.value(QStringLiteral("cavity")).toObject();
    CHECK(c.value(QStringLiteral("openingModel")).toString() == QStringLiteral("geometry"));
    CHECK(c.value(QStringLiteral("surroundings")).toString() == QStringLiteral("free"));
    CHECK(c.value(QStringLiteral("innerLoss")).toObject().value(QStringLiteral("points")).toArray().size() == 2);
    CHECK(c.value(QStringLiteral("openings")).toArray().at(0).toObject().value(QStringLiteral("length")).toDouble() == 0.08);
    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == plan);
    const QJsonObject ohne = alsJson(kp::Bauplan{}).value(QStringLiteral("cavity")).toObject();
    CHECK(ohne.value(QStringLiteral("openingModel")).toString() == QStringLiteral("endCorrection"));
    CHECK(ohne.value(QStringLiteral("surroundings")).toString() == QStringLiteral("wall"));
    CHECK(ohne.contains(QStringLiteral("innerLoss")));
    // Fassung 10 ohne die Felder: Muendungskorrektur und Gueten, Laenge 0
    QJsonObject zehn = json;
    QJsonObject c10 = c;
    c10.remove(QStringLiteral("openingModel"));
    c10.remove(QStringLiteral("surroundings"));
    c10.remove(QStringLiteral("innerLoss"));
    QJsonArray oe10;
    for (const QJsonValue& v : c.value(QStringLiteral("openings")).toArray())
    {
        QJsonObject o = v.toObject();
        o.remove(QStringLiteral("length"));
        oe10.append(o);
    }
    c10[QStringLiteral("openings")] = oe10;
    zehn[QStringLiteral("cavity")] = c10;
    zehn[QStringLiteral("version")] = 10;
    kp::Bauplan alt;
    REQUIRE(ausJson(zehn, alt));
    CHECK(alt.hohlraum.modell == kp::OeffnungsModell::Muendung);
    CHECK(alt.hohlraum.umgebung == kp::OeffnungsUmgebung::Wand);
    CHECK(alt.hohlraum.innen == kp::InnererVerlust{});
    CHECK(alt.hohlraum.oeffnungen[0].laenge == 0.0);
    // unbekannte Werte: nicht raten
    for (const char* feld : {"openingModel", "surroundings"})
    {
        QJsonObject fremd = json;
        QJsonObject cf = c;
        cf[QLatin1String(feld)] = QStringLiteral("baffle");
        fremd[QStringLiteral("cavity")] = cf;
        kp::Bauplan nichts;
        CHECK_FALSE(ausJson(fremd, nichts));
    }
}

TEST_CASE("Loop Koerper KQ71 die Luft der mitgelieferten Harfe gegen Bell 1988 Kap. 4")
{
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(QStringLiteral("asset/loop/instrument_bodies/concert_harp") + QLatin1String(kBauplanEndung), plan),
                    "die Test-Exe aus dem Projekt-Root starten");
    REQUIRE(kp::pruefe(plan).empty());
    REQUIRE(plan.hohlraum.alsRohr());
    REQUIRE(plan.hohlraum.oeffnungen.size() == 6);
    // KQ61 a: Querschnitt ~ Breite² (Bell Abb. 2.7), auf Bells 27 l gestellt; KQ71 a: die Oeffnungen aus ihrer Geometrie
    // als Kolben in der Wand, der innere Verlust aus Bells Gueten mit nur Oeffnung 6 offen
    CHECK(plan.hohlraum.rohr.volumen() == doctest::Approx(0.027).epsilon(1e-6));
    CHECK(plan.hohlraum.ausGeometrie());
    CHECK(plan.hohlraum.umgebung == kp::OeffnungsUmgebung::Wand);
    CHECK(plan.hohlraum.innen.offen == std::vector<int>{5});
    CHECK(plan.hohlraum.innen.punkte.size() == 6);
    for (const kp::Oeffnung& o : plan.hohlraum.oeffnungen) CHECK(o.laenge > 0.1);
    CHECK_FALSE(plan.hohlraum.rohr.quelle.empty());
    CHECK_FALSE(plan.hohlraum.innen.quelle.empty());

    const kp::Luft luft;
    const kp::RohrModen r = kp::rohrModen(plan.hohlraum.rohr, plan.hohlraum.volumen, luft.schall, 5000.0);
    REQUIRE(r.vollstaendig);
    MESSAGE("Rohr: ", r.abschnitte, " Abschnitte, ", r.omega.size() - 1, " Laengsmoden bis 5 kHz");
    CHECK(r.omega.size() - 1 >= 30);
    const kp::LuftraumGroessen gAlle = kp::rohrGroessen(plan, luft, r);
    REQUIRE(gAlle.geometrie);
    REQUIRE(gAlle.innenHz.size() == 6);
    {
        // der innere Verlust: Bells Gueten mit nur Oeffnung 6 offen ohne deren Abstrahlung (Probe S218: Q 11,0 / 10,8 /
        // 12,7 / 24,8 / 32,1 / 38,0)
        std::string zeile;
        for (std::size_t i = 0; i < gAlle.innenHz.size(); ++i)
        {
            CHECK(gAlle.innenKehrwert[i] > 0.0);
            CHECK(1.0 / gAlle.innenKehrwert[i] > plan.hohlraum.innen.punkte[i].q);
            CHECK(1.0 / gAlle.innenKehrwert[i] < 1.15 * plan.hohlraum.innen.punkte[i].q);
            zeile += " " + std::to_string(std::lround(10.0 / gAlle.innenKehrwert[i]) / 10.0).substr(0, 4);
        }
        MESSAGE("innerer Verlust, Q:", zeile);
    }

    // starre Waende: keine Plattenmoden; je Zustand die offenen Oeffnungen mit der Tafel des inneren Verlusts
    const auto luftmoden = [&](const std::vector<int>& offen) {
        kp::Bauplan p = plan;
        p.hohlraum.oeffnungen.clear();
        for (const int i : offen) p.hohlraum.oeffnungen.push_back(plan.hohlraum.oeffnungen[static_cast<std::size_t>(i)]);
        p.hohlraum.innen.offen = {0};
        kp::LuftraumGroessen g = kp::luftraumGroessen(p, luft.dichte, luft.schall);
        g.innenHz = gAlle.innenHz;
        g.innenKehrwert = gAlle.innenKehrwert;
        std::vector<kp::LuftraumMode> m = kp::koppleRohr({}, {}, g, r, luft.dichte);
        m.erase(std::remove_if(m.begin(), m.end(), [](const kp::LuftraumMode& x) { return x.frequenzHz < 20.0; }), m.end());
        return m;
    };

    struct Ziel
    {
        const char* name;
        double hz, q;
        double lageBekannt = 0.0;   // bekannte Abweichung der Lage in % (0: in der Schwelle 3 %)
        double gueteBekannt = 0.0;  // bekannte Abweichung der Guete als Verhaeltnis (0: in der Schwelle 1,5)
    };
    // Bell Abb. 4.8, 4.11-4.15 (am Seitenbild gelesen S216, Harfenbericht §24.1): die Oeffnungen vom Diskant her eine
    // nach der anderen geoeffnet, je Zustand Helmholtz und A1-A5; Gueten nur fuer „6“ und „alle“ (q 0: keine)
    // bekannte Abweichungen (Probe S218, Wand): Lagen ausserhalb 3 % mit ihrem Wert
    const std::vector<std::pair<std::vector<int>, std::vector<Ziel>>> zustaende = {
        {{5},
         {{"6: H", 75.0, 10.0, -4.4}, {"6: A1", 168.0, 10.0}, {"6: A2", 283.0, 12.0}, {"6: A3", 414.0, 23.0}, {"6: A4", 545.0, 30.0},
          {"6: A5", 680.0, 36.0, -3.2}}},
        {{4, 5},
         {{"5-6: H", 92.5, 0.0, 3.8}, {"5-6: A1", 220.0, 0.0}, {"5-6: A2", 364.0, 0.0}, {"5-6: A3", 474.0, 0.0, -3.3},
          {"5-6: A4", 593.0, 0.0, -9.9}, {"5-6: A5", 691.0, 0.0, -4.4}}},
        {{3, 4, 5},
         {{"4-6: H", 100.0, 0.0, 11.5}, {"4-6: A1", 250.0, 0.0, 6.2}, {"4-6: A2", 400.0, 0.0}, {"4-6: A3", 490.0, 0.0, -6.0},
          {"4-6: A4", 600.0, 0.0, -7.2}, {"4-6: A5", 690.0, 0.0, -4.3}}},
        {{2, 3, 4, 5},
         {{"3-6: H", 123.0, 0.0, 6.7}, {"3-6: A1", 290.0, 0.0, 4.7}, {"3-6: A2", 403.0, 0.0}, {"3-6: A3", 507.0, 0.0, -6.8},
          {"3-6: A4", 617.0, 0.0, -8.5}, {"3-6: A5", 707.0, 0.0, -6.0}}},
        {{1, 2, 3, 4, 5},
         {{"2-6: H", 164.0, 0.0}, {"2-6: A1", 277.0, 0.0, 13.3}, {"2-6: A2", 422.0, 0.0, -3.7}, {"2-6: A3", 520.0, 0.0, -8.1},
          {"2-6: A4", 610.0, 0.0, -7.5}, {"2-6: A5", 717.0, 0.0, -6.5}}},
        {{0, 1, 2, 3, 4, 5},
         {{"alle: H", 190.0, 6.0}, {"alle: A1", 292.0, 6.0, 8.2}, {"alle: A2", 423.0, 7.0, -3.8}, {"alle: A3", 530.0, 10.0, -9.5},
          {"alle: A4", 620.0, 15.0, -8.1}, {"alle: A5", 720.0, 30.0, -6.1}}},
    };
    std::vector<kp::LuftraumMode> sechs, alle;
    double summe = 0.0;
    for (const auto& [offen, ziele] : zustaende)
    {
        const std::vector<kp::LuftraumMode> moden = luftmoden(offen);
        REQUIRE(moden.size() >= 6);
        if (offen.size() == 1) sechs = moden;
        if (offen.size() == 6) alle = moden;
        std::string zeile;
        for (std::size_t i = 0; i < 6; ++i)
        {
            const kp::LuftraumMode& m = moden[i];
            const Ziel& z = ziele[i];
            const double lage = 100.0 * (m.frequenzHz / z.hz - 1.0);
            summe += lage * lage;
            const double guete = gueteVon(m);
            const double verhaeltnis = z.q > 0.0 ? std::max(guete / z.q, z.q / guete) : 1.0;
            // die Zuordnung ueber die Zahl der Vorzeichenwechsel des Drucks laengs (Helmholtz 0, A1 1, ...)
            int wechsel = 0;
            double vorher = druckBei(m, r, 0.0);
            for (double y = 0.01; y <= 1.31; y += 0.01)
            {
                const double p = druckBei(m, r, y);
                wechsel += (p > 0.0) != (vorher > 0.0) ? 1 : 0;
                vorher = p;
            }
            CAPTURE(z.name);
            CAPTURE(m.frequenzHz);
            CAPTURE(guete);
            CHECK(wechsel == static_cast<int>(i));
            if (z.lageBekannt == 0.0)
                CHECK(std::abs(lage) <= 3.0);
            else
            {
                CHECK(std::abs(lage) > 3.0);
                CHECK(std::abs(lage - z.lageBekannt) < 1.0);
            }
            if (z.gueteBekannt == 0.0)
                CHECK(verhaeltnis <= 1.5);
            else
            {
                CHECK(verhaeltnis > 1.5);
                CHECK(std::abs(guete / z.q - z.gueteBekannt) < 0.15);
            }
            zeile += std::string(" ") + z.name + " " + std::to_string(std::lround(10.0 * m.frequenzHz) / 10.0).substr(0, 5) + " Hz (" +
                     std::to_string(std::lround(10.0 * lage) / 10.0).substr(0, 5) + " %)" +
                     (z.q > 0.0 ? " Q " + std::to_string(std::lround(10.0 * guete) / 10.0).substr(0, 4) : std::string()) + ";";
        }
        MESSAGE(zeile);
    }
    // die 36 Lagen zusammen (Probe S218, Wand: rms 6,1 %; gebaut S215 mit Muendung 1,83: 7,2 %)
    const double rms = std::sqrt(summe / 36.0);
    MESSAGE("36 Lagen gegen Bell: rms ", rms, " %");
    CHECK(rms < 6.5);
    // mit nur Oeffnung 6 offen traegt jede Mode Bells Guete (der innere Verlust ist an ihnen gestellt; die Tafel liegt
    // an Bells Frequenzen, die Moden auf wenigen Prozent daneben)
    for (std::size_t i = 0; i < 6; ++i)
    {
        CAPTURE(i);
        CHECK(gueteVon(sechs[i]) == doctest::Approx(zustaende[0].second[i].q).epsilon(0.1));
    }

    // KQ60 a: die Helmholtz-Mode mit allen Oeffnungen bei 190 ± 3 Hz (Bell S. 85, Abb. 4.8 vi), ohne angepassten Wert
    CHECK(std::abs(alle[0].frequenzHz - 190.0) <= 3.0);
    // ihr Druck faellt zum Diskant (Bell Abb. 4.8 vi: rund 18 dB von 10 bis 120 cm); mit Oeffnung 6 allein steigt er
    const double abfall = 20.0 * std::log10(std::abs(druckBei(alle[0], r, 0.1) / druckBei(alle[0], r, 1.2)));
    const double anstieg = 20.0 * std::log10(std::abs(druckBei(sechs[0], r, 1.2) / druckBei(sechs[0], r, 0.1)));
    MESSAGE("Helmholtz alle offen: Abfall 10 -> 120 cm ", abfall, " dB (Bell rund 18); Oeffnung 6 allein: Anstieg ", anstieg, " dB");
    CHECK(std::abs(abfall - 18.0) <= 3.0);
    CHECK(anstieg > 0.0);
}
