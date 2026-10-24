/**
 ****************************************************************************************
 * @file   test_LoopKoerperBauplan.cpp
 * @brief  Loop, Stufe 31c-1: der Bauplan eines Instrumentkoerpers und seine Datei
 *         `.lvinstbodyplan` (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik
 *         S147: BM1 a, BM3 a, BM3-2 a, BM5 a, GB1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Dateien aus dem Repo, ohne Qt-Anwendung.
 *
 * - Die Geometrie der Bibliothek `InstrumentBody`: Flaeche, innen, Abstand zur Linie,
 *   das Dickenraster, die Masse von Platte und Leisten an Faellen, die sich von Hand
 *   rechnen lassen.
 * - Isotrope Werkstoffe (Glas, GB1 a): G = E / (2 (1 + ν)).
 * - `pruefe` findet, was an einem Bauplan nicht stimmt.
 * - Die Datei: Hin- und Rueckweg gleich, jedes Feld geschrieben, NaN im Raster als null,
 *   falsches Format und zu neue Fassung abgewiesen; Fassung 1 (ohne Hoerorte) lesbar.
 * - Hoerorte (S148, BM6 a, BM6-1 a): in `pruefe`, in der Datei, beim Steinway D das Stereopaar
 *   12 cm ueber den Saiten an den Stegpunkten von MIDI 43 und 86.
 * - Der mitgelieferte Steinway D (BM2 a, `make_steinway_d.py`), vom Arbeitsordner aus gelesen
 *   (die Test-Exe aus dem Projekt-Root starten): in sich stimmig, 88 Tasten, jeder Stegpunkt
 *   auf dem Steg, Dicken in den Spannen von RT-0425 §2.1, Masse plausibel.
 * - Der MAESSTRO-Prueffall (S150, PF1 a, PF2 a; Testdaten unter `koerper/`, aus
 *   `make_steinway_d.py --maesstro`): Umriss, Rippenlinien und Stegpunkte wie im Steinway D,
 *   Platte, Stege, Rippen und Werkstoffe nach MAESSTRO Tab. 1–2, der Steg geteilt bei 40/41.
 * - Die Icons der beiden Endungen (IC1 a) liegen, wo `dateiArtIcon` sie sucht
 *   (`lumiviz_<endung>.ico`), ebenfalls vom Arbeitsordner aus.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Bauplan.hpp"
#include "InstrumentBody/Platte.hpp"
#include "loop/KoerperBauplan.hpp"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

#include <cmath>
#include <limits>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

/// Ein Bauplan, an dem sich alles von Hand nachrechnen laesst: eine Platte 1 m × 0,5 m,
/// 8 mm Fichte (400 kg/m³), eine Rippe 1 m lang, 20 × 10 mm, ein Steg aus Glas.
kp::Bauplan einfacherPlan()
{
    kp::Bauplan plan;
    plan.titel = "Pruefplatte";
    plan.beschreibung = "Rechteck fuer die Tests";
    plan.quelle = "von Hand";
    kp::Material fichte;
    fichte.name = "spruce";
    fichte.dichte = 400.0;
    fichte.ex = 11.0e9;
    fichte.ey = 0.65e9;
    fichte.gxy = 0.66e9;
    fichte.gxz = 1.2e9;
    fichte.gyz = 0.042e9;
    fichte.nuxy = 0.26;
    fichte.verlustfaktor = 0.02;
    fichte.quelle = "MAESSTRO Tab. 2 (Dichte gerundet)";
    kp::Material glas;
    glas.name = "glass";
    glas.art = kp::MaterialArt::Isotrop;
    glas.dichte = 2500.0;
    glas.ex = 70.0e9;
    glas.nuxy = 0.22;
    plan.materialien = {fichte, glas};
    plan.platte.material = "spruce";
    plan.platte.faserWinkelGrad = 135.0;
    plan.platte.umriss = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 0.5}, {0.0, 0.5}};
    plan.platte.dicke = 0.008;
    plan.rippen.push_back({"rib 1", "spruce", {{0.0, 0.25}, {1.0, 0.25}}, {0.02, 0.02}, {0.01, 0.01}});
    plan.stege.push_back({"bridge", "glass", {{0.5, 0.0}, {0.5, 0.5}}, {0.03, 0.03}, {0.03, 0.03}});
    plan.stegpunkte = {{60, {0.5, 0.1}, "von Hand"}, {72, {0.5, 0.3}, "von Hand"}};
    plan.hoerorte = {{"left", {0.3, 0.2}, 0.15, "von Hand"}, {"right", {0.7, 0.2}, 0.15, "von Hand"}};
    return plan;
}

}  // namespace

TEST_CASE("Loop Koerper 31c-1 Geometrie")
{
    const std::vector<kp::Punkt2> rechteck = {{0.0, 0.0}, {2.0, 0.0}, {2.0, 1.0}, {0.0, 1.0}};
    CHECK(kp::flaeche(rechteck) == doctest::Approx(2.0));
    // gegen den Uhrzeiger und mit ihm dieselbe Flaeche
    CHECK(kp::flaeche({{0.0, 0.0}, {0.0, 1.0}, {2.0, 1.0}, {2.0, 0.0}}) == doctest::Approx(2.0));
    CHECK(kp::flaeche({{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}}) == doctest::Approx(0.5));

    CHECK(kp::innen(rechteck, {1.0, 0.5}));
    CHECK_FALSE(kp::innen(rechteck, {2.5, 0.5}));
    CHECK_FALSE(kp::innen(rechteck, {1.0, -0.1}));

    const std::vector<kp::Punkt2> knick = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}};
    CHECK(kp::abstandZurLinie({0.5, 0.2}, knick) == doctest::Approx(0.2));
    CHECK(kp::abstandZurLinie({1.3, 0.5}, knick) == doctest::Approx(0.3));
    CHECK(kp::abstandZurLinie({-0.3, -0.4}, knick) == doctest::Approx(0.5));  // am Anfang vorbei
    CHECK(kp::laenge(knick) == doctest::Approx(2.0));
}

TEST_CASE("Loop Koerper 31c-1 Dickenraster")
{
    kp::Raster r;
    r.ursprung = {0.0, 0.0};
    r.schritt = 1.0;
    r.nx = 2;
    r.ny = 2;
    r.werte = {1.0, 3.0, 5.0, 7.0};
    CHECK(r.wertBei({0.0, 0.0}) == doctest::Approx(1.0));
    CHECK(r.wertBei({1.0, 1.0}) == doctest::Approx(7.0));
    CHECK(r.wertBei({0.5, 0.5}) == doctest::Approx(4.0));
    CHECK(r.wertBei({0.5, 0.0}) == doctest::Approx(2.0));
    // ein fehlender Nachbar zaehlt nicht
    r.werte[3] = std::numeric_limits<double>::quiet_NaN();
    CHECK(r.wertBei({0.5, 0.5}) == doctest::Approx(3.0));
    // weit draussen: kein Wert, die Platte nimmt ihre feste Dicke
    CHECK(std::isnan(r.wertBei({5.0, 5.0})));
    kp::Platte p;
    p.dicke = 0.006;
    p.dickeRaster = r;
    CHECK(p.dickeBei({5.0, 5.0}) == doctest::Approx(0.006));
    CHECK(p.dickeBei({0.0, 0.0}) == doctest::Approx(1.0));
}

TEST_CASE("Loop Koerper 31c-1 Masse und Werkstoffe")
{
    const kp::Bauplan plan = einfacherPlan();
    // Platte 1 × 0,5 × 0,008 m³ · 400 = 1,6 kg (das Raster der Summe trifft das Rechteck genau)
    CHECK(kp::plattenMasse(plan) == doctest::Approx(1.6).epsilon(1e-6));
    // Rippe 1 m · 0,02 · 0,01 · 400 = 0,08 kg; Steg 0,5 m · 0,03 · 0,03 · 2500 = 1,125 kg
    CHECK(kp::masse(plan.rippen[0], 400.0) == doctest::Approx(0.08));
    CHECK(kp::masse(plan) == doctest::Approx(1.6 + 0.08 + 1.125).epsilon(1e-6));
    // eine Leiste, die sich verjuengt: Querschnitt im Mittel
    const kp::Leiste spitz{"taper", "spruce", {{0.0, 0.0}, {1.0, 0.0}}, {0.02, 0.0}, {0.01, 0.01}};
    CHECK(kp::masse(spitz, 400.0) == doctest::Approx(0.04));

    // Glas (GB1 a): isotrop, die Schubmoduln aus E und ν
    const kp::Material glas = kp::vervollstaendigt(*kp::findeMaterial(plan, "glass"));
    CHECK(glas.ey == doctest::Approx(70.0e9));
    CHECK(glas.gxy == doctest::Approx(70.0e9 / (2.0 * 1.22)));
    CHECK(glas.gxz == doctest::Approx(glas.gxy));
    CHECK(glas.gyz == doctest::Approx(glas.gxy));
    // Holz bleibt, wie es ist
    const kp::Material fichte = kp::vervollstaendigt(*kp::findeMaterial(plan, "spruce"));
    CHECK(fichte.ey == doctest::Approx(0.65e9));
    CHECK(kp::findeMaterial(plan, "maple") == nullptr);
}

TEST_CASE("Loop Koerper 31c-1 pruefe")
{
    CHECK(kp::pruefe(einfacherPlan()).empty());

    const auto befundeMit = [](auto aendern) {
        kp::Bauplan plan = einfacherPlan();
        aendern(plan);
        return kp::pruefe(plan);
    };
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.platte.material = "maple"; }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.platte.umriss.resize(2); }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.platte.dicke = 0.0; }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.materialien.push_back(p.materialien[0]); }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.materialien[0].dichte = 0.0; }).empty());
    // isotrop: ν ab 0,5 geht nicht; orthotrop: 1 − ν_xy·ν_yx muss positiv bleiben
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.materialien[1].nuxy = 0.5; }).empty());
    CHECK(befundeMit([](kp::Bauplan& p) { p.materialien[0].nuxy = 0.9; }).empty());  // 0,81 · 0,65/11 < 1
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.materialien[0].nuxy = 5.0; }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.rippen[0].hoehe.pop_back(); }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.stege[0].breite[0] = 0.0; }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.stege[0].material = "oak"; }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.stegpunkte[1].taste = 60; }).empty());
    // Hoerorte (BM6 a): Name eindeutig, ueber der Platte
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.hoerorte[1].name = "left"; }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.hoerorte[0].name.clear(); }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) { p.hoerorte[0].hoehe = 0.0; }).empty());
    CHECK(befundeMit([](kp::Bauplan& p) { p.hoerorte.clear(); }).empty());
    CHECK_FALSE(befundeMit([](kp::Bauplan& p) {
                    p.platte.dickeRaster.nx = 2;
                    p.platte.dickeRaster.ny = 2;
                    p.platte.dickeRaster.schritt = 0.1;
                    p.platte.dickeRaster.werte = {0.008};
                }).empty());
}

TEST_CASE("Loop Koerper 31c-1 Datei")
{
    kp::Bauplan plan = einfacherPlan();
    plan.platte.dickeRaster.ursprung = {0.05, 0.05};
    plan.platte.dickeRaster.schritt = 0.1;
    plan.platte.dickeRaster.nx = 2;
    plan.platte.dickeRaster.ny = 1;
    plan.platte.dickeRaster.werte = {0.009, std::numeric_limits<double>::quiet_NaN()};

    const QJsonObject json = alsJson(plan);
    CHECK(json.value(QStringLiteral("format")).toString() == QLatin1String(kBauplanFormat));
    CHECK(json.value(QStringLiteral("version")).toInt() == kp::kBauplanFassung);
    // jedes Feld steht da, auch ein leeres
    for (const char* feld : {"title", "description", "source", "units", "materials", "plate", "ribs", "bridges", "bridgePoints",
                             "listeningPoints"})
        CHECK_MESSAGE(json.contains(QLatin1String(feld)), feld);
    const QJsonObject hoerort = json.value(QStringLiteral("listeningPoints")).toArray()[0].toObject();
    for (const char* feld : {"name", "x", "y", "z", "source"}) CHECK_MESSAGE(hoerort.contains(QLatin1String(feld)), feld);
    const QJsonObject platte = json.value(QStringLiteral("plate")).toObject();
    for (const char* feld : {"material", "grainAngle", "outline", "thickness", "thicknessGrid"})
        CHECK_MESSAGE(platte.contains(QLatin1String(feld)), feld);
    const QJsonObject glas = json.value(QStringLiteral("materials")).toArray()[1].toObject();
    CHECK(glas.value(QStringLiteral("kind")).toString() == QLatin1String("isotropic"));
    CHECK(glas.contains(QStringLiteral("lossFactor")));
    // NaN steht als null
    const QJsonArray werte = platte.value(QStringLiteral("thicknessGrid")).toObject().value(QStringLiteral("values")).toArray();
    REQUIRE(werte.size() == 2);
    CHECK(werte[1].isNull());

    kp::Bauplan zurueck;
    REQUIRE(ausJson(json, zurueck));
    CHECK(zurueck == plan);

    // ein leerer Bauplan geht auch hin und zurueck, mit allen Feldern
    const QJsonObject leer = alsJson(kp::Bauplan{});
    CHECK(leer.value(QStringLiteral("plate")).toObject().contains(QStringLiteral("thicknessGrid")));
    kp::Bauplan leerZurueck;
    REQUIRE(ausJson(leer, leerZurueck));
    CHECK(leerZurueck == kp::Bauplan{});

    // falsches Format: abgewiesen, der Bauplan bleibt
    QJsonObject fremd = json;
    fremd[QStringLiteral("format")] = QStringLiteral("lumiviz-instrument");
    bool zuNeu = true;
    kp::Bauplan bleibt = einfacherPlan();
    CHECK_FALSE(ausJson(fremd, bleibt, &zuNeu));
    CHECK_FALSE(zuNeu);
    CHECK(bleibt == einfacherPlan());

    // Fassung 1 (vor S148) kennt keine Hoerorte: lesbar, ohne Hoerorte
    QJsonObject alt = json;
    alt[QStringLiteral("version")] = 1;
    alt.remove(QStringLiteral("listeningPoints"));
    kp::Bauplan ausAlt;
    REQUIRE(ausJson(alt, ausAlt, &zuNeu));
    CHECK_FALSE(zuNeu);
    CHECK(ausAlt.hoerorte.empty());
    kp::Bauplan ohneHoerorte = plan;
    ohneHoerorte.hoerorte.clear();
    CHECK(ausAlt == ohneHoerorte);

    // eine Fassung, die es noch nicht gibt
    QJsonObject neuer = json;
    neuer[QStringLiteral("version")] = kp::kBauplanFassung + 1;
    CHECK_FALSE(ausJson(neuer, bleibt, &zuNeu));
    CHECK(zuNeu);

    // ein kaputter Punkt im Umriss
    QJsonObject kaputt = json;
    QJsonObject kaputtePlatte = platte;
    kaputtePlatte[QStringLiteral("outline")] = QJsonArray{QJsonArray{0.0}};
    kaputt[QStringLiteral("plate")] = kaputtePlatte;
    CHECK_FALSE(ausJson(kaputt, bleibt));
}

TEST_CASE("Loop Koerper 31c-1 Steinway D")
{
    kp::Bauplan plan;
    bool zuNeu = false;
    REQUIRE_MESSAGE(ladeBauplan(QStringLiteral("asset/loop/instrument_bodies/steinway_d") + QLatin1String(kBauplanEndung), plan, &zuNeu),
                    "die Test-Exe aus dem Projekt-Root starten");
    CHECK_FALSE(zuNeu);
    const std::vector<std::string> befunde = kp::pruefe(plan);
    for (const std::string& b : befunde) MESSAGE(b);
    CHECK(befunde.empty());

    // Holz nach Chabassier, Joly & Chaigne 2013, Tab. I
    const kp::Material* fichte = kp::findeMaterial(plan, "spruce");
    const kp::Material* buche = kp::findeMaterial(plan, "beech");
    REQUIRE(fichte);
    REQUIRE(buche);
    CHECK(fichte->dichte == doctest::Approx(380.0));
    CHECK(fichte->ex == doctest::Approx(11.0e9));
    CHECK(buche->dichte == doctest::Approx(750.0));
    CHECK(plan.platte.material == "spruce");

    // 88 Tasten, A0 … C8; C1 … B7 aus Anhang A, die vier anderen verlaengert
    REQUIRE(plan.stegpunkte.size() == 88);
    CHECK(plan.stegpunkte.front().taste == 21);
    CHECK(plan.stegpunkte.back().taste == 108);
    int ausAnhang = 0;
    for (const kp::Stegpunkt& s : plan.stegpunkte) ausAnhang += s.quelle.rfind("RT-0425", 0) == 0 ? 1 : 0;
    CHECK(ausAnhang == 84);
    // Stichproben aus Anhang A (RT-0425, gedruckt S. 11 und 15)
    const auto ort = [&](int taste) {
        for (const kp::Stegpunkt& s : plan.stegpunkte)
            if (s.taste == taste) return s.ort;
        return kp::Punkt2{};
    };
    CHECK(ort(24).x == doctest::Approx(0.40));  // C1
    CHECK(ort(24).y == doctest::Approx(1.69));
    CHECK(ort(96).x == doctest::Approx(1.24));  // C7
    CHECK(ort(96).y == doctest::Approx(0.02));
    CHECK(ort(107).y == doctest::Approx(-0.03));  // B7: unter y = 0, der Umriss reicht dorthin

    // jeder Stegpunkt liegt auf dem Steg (halbe Breite) und im Umriss
    REQUIRE(plan.stege.size() == 1);
    const kp::Leiste& steg = plan.stege[0];
    double groesster = 0.0;
    for (const kp::Stegpunkt& s : plan.stegpunkte)
    {
        const double d = kp::abstandZurLinie(s.ort, steg.linie);
        groesster = std::max(groesster, d);
        CHECK_MESSAGE(d <= steg.breite.front() / 2.0, "Taste ", s.taste, ": ", d, " m neben dem Steg");
        CHECK_MESSAGE(kp::innen(plan.platte.umriss, s.ort), "Taste ", s.taste);
    }
    MESSAGE("groesster Abstand eines Stegpunkts zur Steglinie: ", groesster * 1e3, " mm");

    // Dicken gesamt in den Spannen von RT-0425 §2.1 (Platte 6–9 mm — das Bild zeigt bis 9,9;
    // Steg 29–69 mm; Rippen 9–35 mm), mit dem Raster der Auswertung (2 cm) als Spiel
    double plMin = 1.0, plMax = 0.0;
    for (const double v : plan.platte.dickeRaster.werte)
    {
        if (std::isnan(v)) continue;
        plMin = std::min(plMin, v);
        plMax = std::max(plMax, v);
    }
    CHECK(plMin >= 0.006 - 1e-4);
    CHECK(plMax <= 0.010);
    const auto gesamt = [&](const kp::Leiste& l, double& lo, double& hi) {
        for (std::size_t k = 0; k < l.linie.size(); ++k)
        {
            const double g = l.hoehe[k] + plan.platte.dickeBei(l.linie[k]);
            lo = std::min(lo, g);
            hi = std::max(hi, g);
        }
    };
    double stLo = 1.0, stHi = 0.0;
    gesamt(steg, stLo, stHi);
    CHECK(stLo >= 0.029 - 0.003);
    CHECK(stHi <= 0.069 + 0.003);
    double riLo = 1.0, riHi = 0.0;
    for (const kp::Leiste& r : plan.rippen) gesamt(r, riLo, riHi);
    CHECK(riLo >= 0.009 - 0.002);
    CHECK(riHi <= 0.035 + 0.003);
    CHECK(plan.rippen.size() == 16);

    // Flaeche und Masse: rund 2 m²; Plath 2019 Tab. 6.2 misst am fertigen Boden eines
    // Konzertfluegels 13,70 kg — RT-0425 zeichnet die Rippen kuerzer (0,3 … 0,8 m), deshalb weniger
    const double flaeche = kp::flaeche(plan.platte.umriss);
    const double masse = kp::masse(plan);
    MESSAGE("Flaeche ", flaeche, " m², Masse ", masse, " kg (Platte ", kp::plattenMasse(plan), " kg)");
    CHECK(flaeche == doctest::Approx(2.05).epsilon(0.03));
    CHECK(masse > 10.0);
    CHECK(masse < 15.0);

    // Hoerorte (BM6-1 a): ein Stereopaar 12 cm ueber den Saiten, ueber den Stegpunkten von MIDI 43
    // und 86; z ueber der Mittelflaeche = halbe Plattendicke + Hoehe des Stegs dort + 12 cm
    REQUIRE(plan.hoerorte.size() == 2);
    const std::pair<const char*, int> paar[] = {{"left", 43}, {"right", 86}};
    for (std::size_t i = 0; i < 2; ++i)
    {
        const kp::Hoerort& h = plan.hoerorte[i];
        CAPTURE(h.name);
        CHECK(h.name == paar[i].first);
        CHECK(h.ort.x == doctest::Approx(ort(paar[i].second).x));
        CHECK(h.ort.y == doctest::Approx(ort(paar[i].second).y));
        const kp::AufLeiste auf = kp::aufLeiste(steg, h.ort);
        CHECK(auf.drauf);
        // das Skript nimmt die Plattendicke der naechsten Rasterzelle, die Bibliothek bilinear: 1 mm Spiel
        const double soll = plan.platte.dickeBei(h.ort) / 2.0 + auf.hoehe + 0.12;
        CHECK_MESSAGE(std::abs(h.hoehe - soll) < 0.001, "z ", h.hoehe, " m, erwartet ", soll, " m");
    }
}

TEST_CASE("Loop Koerper 31c-6 MAESSTRO Prueffall")
{
    // PF1 a, PF2 a: der Steinway D mit den Vereinfachungen von MAESSTRO (Elie, Cotte & Boutillon 2022,
    // Tab. 1 und 2), Testdaten aus `make_steinway_d.py --maesstro`
    kp::Bauplan plan;
    kp::Bauplan steinway;
    REQUIRE_MESSAGE(ladeBauplan(QStringLiteral("projects/apps/LumiViz/tests/unit/UnitTests/koerper/maesstro_rp") +
                                    QLatin1String(kBauplanEndung),
                                plan),
                    "die Test-Exe aus dem Projekt-Root starten");
    REQUIRE(ladeBauplan(QStringLiteral("asset/loop/instrument_bodies/steinway_d") + QLatin1String(kBauplanEndung), steinway));
    const std::vector<std::string> befunde = kp::pruefe(plan);
    for (const std::string& b : befunde) MESSAGE(b);
    CHECK(befunde.empty());

    // vom Steinway D unveraendert: Umriss, Rippenlinien, Stegpunkte
    CHECK(plan.platte.umriss == steinway.platte.umriss);
    CHECK(plan.stegpunkte == steinway.stegpunkte);
    REQUIRE(plan.rippen.size() == steinway.rippen.size());
    for (std::size_t k = 0; k < plan.rippen.size(); ++k) CHECK(plan.rippen[k].linie == steinway.rippen[k].linie);

    // Tab. 2: Sitka und Buche, je η = 2 %
    const kp::Material* sitka = kp::findeMaterial(plan, "sitka");
    const kp::Material* buche = kp::findeMaterial(plan, "beech");
    REQUIRE(sitka);
    REQUIRE(buche);
    CHECK(sitka->dichte == doctest::Approx(380.0));
    CHECK(sitka->ex == doctest::Approx(11.0e9));
    CHECK(sitka->gyz == doctest::Approx(0.042e9));
    CHECK(buche->dichte == doctest::Approx(750.0));
    CHECK(buche->ey == doctest::Approx(2.24e9));
    CHECK(sitka->verlustfaktor == doctest::Approx(0.02));
    CHECK(buche->verlustfaktor == doctest::Approx(0.02));

    // Tab. 1: Platte 8 mm ueberall, Faser 135°
    CHECK(plan.platte.material == "sitka");
    CHECK(plan.platte.faserWinkelGrad == doctest::Approx(135.0));
    CHECK(plan.platte.dickeRaster.leer());
    CHECK(plan.platte.dickeBei({0.5, 1.0}) == doctest::Approx(0.008));

    // zwei Stege (PF2 a): der Bass-Steg 60 × 32 mm traegt die Tasten 21–40, der Diskant-Steg
    // 20 × 32 mm die Tasten 41–108; jeder Stegpunkt auf seinem Steg, der Haken entfaellt
    REQUIRE(plan.stege.size() == 2);
    const kp::Leiste& bass = plan.stege[0];
    const kp::Leiste& diskant = plan.stege[1];
    CHECK(bass.name == "bass bridge");
    CHECK(diskant.name == "treble bridge");
    for (const double h : bass.hoehe) CHECK(h == doctest::Approx(0.060));
    for (const double h : diskant.hoehe) CHECK(h == doctest::Approx(0.020));
    for (const kp::Leiste* l : {&bass, &diskant})
    {
        CHECK(l->material == "beech");
        for (const double b : l->breite) CHECK(b == doctest::Approx(0.032));
    }
    for (const kp::Stegpunkt& s : plan.stegpunkte)
    {
        const kp::Leiste& eigener = s.taste <= 40 ? bass : diskant;
        const double d = kp::abstandZurLinie(s.ort, eigener.linie);
        CHECK_MESSAGE(d <= 0.016, "Taste ", s.taste, ": ", d, " m neben ihrem Steg");
    }
    double abstand = 1.0;
    for (const kp::Punkt2& p : bass.linie) abstand = std::min(abstand, kp::abstandZurLinie(p, diskant.linie));
    MESSAGE("kleinster Abstand der beiden Stege: ", abstand * 1e3, " mm");
    CHECK(abstand > 0.05);

    // Tab. 1: Rippen ohne Verjuengung, von Bass nach Diskant 27 → 17 mm hoch, 25 → 21 mm breit
    REQUIRE(plan.rippen.size() == 16);
    for (std::size_t k = 0; k < plan.rippen.size(); ++k)
    {
        const kp::Leiste& r = plan.rippen[k];
        const double t = static_cast<double>(k) / (plan.rippen.size() - 1);
        CAPTURE(r.name);
        CHECK(r.material == "sitka");
        for (const double h : r.hoehe) CHECK(h == doctest::Approx(0.027 - 0.010 * t).epsilon(1e-4));
        for (const double b : r.breite) CHECK(b == doctest::Approx(0.025 - 0.004 * t).epsilon(1e-4));
    }

    // Hoerorte an denselben Stellen; z = halbe Platte + Diskant-Steg + 12 cm (beide Tasten im Diskant)
    REQUIRE(plan.hoerorte.size() == 2);
    for (std::size_t i = 0; i < 2; ++i)
    {
        CHECK(plan.hoerorte[i].ort == steinway.hoerorte[i].ort);
        CHECK(plan.hoerorte[i].hoehe == doctest::Approx(0.004 + 0.020 + 0.12));
    }
    MESSAGE("Masse ", kp::masse(plan), " kg (Platte ", kp::plattenMasse(plan), " kg)");
}

TEST_CASE("Loop Koerper 31c-1 Icons")
{
    // dateiArtIcon(<endung>) laedt `lumiviz_<endung>.ico` aus dem Ordner der Format-Icons
    for (const char* endung : {kBauplanEndung, kKoerperEndung})
    {
        const QString datei = QStringLiteral("asset/img/logo/icons/lumiviz_") + QLatin1String(endung).mid(1) + QStringLiteral(".ico");
        CHECK_MESSAGE(QFileInfo::exists(datei), datei.toStdString(), " (die Test-Exe aus dem Projekt-Root starten)");
    }
}
