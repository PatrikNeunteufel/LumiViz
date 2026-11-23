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
 * - KQ64 a: die Luft der mitgelieferten Harfe bei starren Waenden gegen Bell 1988 Kap. 4 (Kasten A im Sand; Abb. 4.8,
 *   4.11–4.15, am Seitenbild gelesen S215): mit allen Oeffnungen die Helmholtz-Mode bei 190 ± 1,5 Hz und ihr Abfall
 *   zum Diskant (Bell rund 18 dB von 10 bis 120 cm, auf 3 dB); mit Oeffnung 6 allein A2–A5 auf 3 %; die zwoelf Gueten
 *   auf den Faktor 1,5. Was darueber liegt, steht als bekannte Abweichung mit Wert — rot, wenn sie waechst oder
 *   verschwindet: mit allen Oeffnungen A2–A5 (Ursache offen, Harfenbericht §23.2), die Helmholtz-Mode mit Oeffnung 6
 *   allein und eine Guete.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Luftraum.hpp"
#include "loop/KoerperBauplan.hpp"

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

/// Die Guete einer Luftmode bei starren Waenden (KQ63 a).
double gueteVon(const kp::LuftraumMode& m, const kp::Bauplan& plan)
{
    return 1.0 / ((m.luftAnteil - m.rohrAnteil) / plan.hohlraum.guete + m.rohrAnteil / plan.hohlraum.rohr.guete);
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

TEST_CASE("Loop Koerper KQ64 die Luft der mitgelieferten Harfe gegen Bell 1988 Kap. 4")
{
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(QStringLiteral("asset/loop/instrument_bodies/concert_harp") + QLatin1String(kBauplanEndung), plan),
                    "die Test-Exe aus dem Projekt-Root starten");
    REQUIRE(kp::pruefe(plan).empty());
    REQUIRE(plan.hohlraum.alsRohr());
    REQUIRE(plan.hohlraum.oeffnungen.size() == 6);
    // KQ61 a: Querschnitt ~ Breite² (Bell Abb. 2.7), auf Bells 27 l gestellt; KQ63 a: zwei Gueten
    CHECK(plan.hohlraum.rohr.volumen() == doctest::Approx(0.027).epsilon(1e-6));
    CHECK(plan.hohlraum.guete == doctest::Approx(4.4));
    CHECK(plan.hohlraum.rohr.guete == doctest::Approx(28.0));
    CHECK_FALSE(plan.hohlraum.rohr.quelle.empty());

    const kp::Luft luft;
    const kp::RohrModen r = kp::rohrModen(plan.hohlraum.rohr, plan.hohlraum.volumen, luft.schall, 5000.0);
    REQUIRE(r.vollstaendig);
    MESSAGE("Rohr: ", r.abschnitte, " Abschnitte, ", r.omega.size() - 1, " Laengsmoden bis 5 kHz");
    CHECK(r.omega.size() - 1 >= 30);

    // starre Waende: keine Plattenmoden
    const auto luftmoden = [&](const kp::Bauplan& p) {
        const kp::LuftraumGroessen g = kp::luftraumGroessen(p, luft.dichte, luft.schall);
        std::vector<kp::LuftraumMode> m = kp::koppleRohr({}, {}, g, r, luft.dichte);
        m.erase(std::remove_if(m.begin(), m.end(), [](const kp::LuftraumMode& x) { return x.frequenzHz < 20.0; }), m.end());
        return m;
    };
    kp::Bauplan nurSechs = plan;
    nurSechs.hohlraum.oeffnungen = {plan.hohlraum.oeffnungen[5]};  // Oeffnung 6 im Bodenblock
    const std::vector<kp::LuftraumMode> sechs = luftmoden(nurSechs), alle = luftmoden(plan);
    REQUIRE(sechs.size() >= 6);
    REQUIRE(alle.size() >= 6);

    struct Ziel
    {
        const char* name;
        double hz, q;
        double lageBekannt = 0.0;   // bekannte Abweichung der Lage in % (0: in der Schwelle 3 %)
        double gueteBekannt = 0.0;  // bekannte Abweichung der Guete als Verhaeltnis (0: in der Schwelle 1,5)
    };
    // Bell Abb. 4.8 i, 4.11-4.15 i (nur Oeffnung 6) und 4.8 vi, 4.11-4.15 vi (alle offen)
    // bekannte Abweichungen (S215): mit Oeffnung 6 allein die Helmholtz-Mode −8,4 % mit Q 6,3 statt 10, A2 mit Q 20
    // statt 12, A5 −3,2 %; mit allen Oeffnungen A2–A5 −11,5 / −13,2 / −8,9 / −5,9 % (Harfenbericht §23.2)
    const Ziel zielSechs[] = {{"6: Helmholtz", 75.0, 10.0, -8.4, 0.63},
                              {"6: A1", 168.0, 10.0},
                              {"6: A2", 283.0, 12.0, 0.0, 1.66},
                              {"6: A3", 414.0, 23.0},
                              {"6: A4", 545.0, 30.0},
                              {"6: A5", 680.0, 36.0, -3.2}};
    const Ziel zielAlle[] = {{"alle: Helmholtz", 190.0, 6.0},
                             {"alle: A1", 292.0, 6.0},
                             {"alle: A2", 423.0, 7.0, -11.5},
                             {"alle: A3", 530.0, 10.0, -13.2},
                             {"alle: A4", 620.0, 15.0, -8.9},
                             {"alle: A5", 720.0, 30.0, -5.9}};
    const auto pruefe = [&](const std::vector<kp::LuftraumMode>& moden, const Ziel* ziele) {
        std::string zeile;
        for (std::size_t i = 0; i < 6; ++i)
        {
            const kp::LuftraumMode& m = moden[i];
            const Ziel& z = ziele[i];
            const double lage = 100.0 * (m.frequenzHz / z.hz - 1.0);
            const double guete = gueteVon(m, plan);
            const double verhaeltnis = std::max(guete / z.q, z.q / guete);
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
                CHECK(std::abs(lage - z.lageBekannt) < 1.5);
            }
            if (z.gueteBekannt == 0.0)
                CHECK(verhaeltnis <= 1.5);
            else
            {
                CHECK(verhaeltnis > 1.5);
                CHECK(std::abs(guete / z.q - z.gueteBekannt) < 0.15);
            }
            zeile += std::string(" ") + z.name + " " + std::to_string(std::lround(10.0 * m.frequenzHz) / 10.0).substr(0, 5) + " Hz (" +
                     std::to_string(std::lround(10.0 * lage) / 10.0).substr(0, 5) + " %) Q " + std::to_string(std::lround(guete)) + ";";
        }
        MESSAGE(zeile);
    };
    pruefe(sechs, zielSechs);
    pruefe(alle, zielAlle);

    // KQ60 a: die Helmholtz-Mode mit allen Oeffnungen bei 190 ± 1,5 Hz (Bell S. 85, Abb. 4.8 vi)
    CHECK(std::abs(alle[0].frequenzHz - 190.0) <= 1.5);
    // ihr Druck faellt zum Diskant (Bell Abb. 4.8 vi: rund 18 dB von 10 bis 120 cm); mit Oeffnung 6 allein steigt er
    const double abfall = 20.0 * std::log10(std::abs(druckBei(alle[0], r, 0.1) / druckBei(alle[0], r, 1.2)));
    const double anstieg = 20.0 * std::log10(std::abs(druckBei(sechs[0], r, 1.2) / druckBei(sechs[0], r, 0.1)));
    MESSAGE("Helmholtz alle offen: Abfall 10 -> 120 cm ", abfall, " dB (Bell rund 18); Oeffnung 6 allein: Anstieg ", anstieg, " dB");
    CHECK(std::abs(abfall - 18.0) <= 3.0);
    CHECK(anstieg > 0.0);
}
