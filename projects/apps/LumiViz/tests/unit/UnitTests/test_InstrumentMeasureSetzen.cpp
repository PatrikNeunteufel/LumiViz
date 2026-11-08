/**
 * @file test_InstrumentMeasureSetzen.cpp
 * @note Uebernommen aus IC `rechnung/setzen/test_setzen.cpp` (bb7e6d6, IC Session 5; Instrumente_IC_Konzept.md 34e-4,
 *       Journal J24): die Faelle wortgleich, mit denselben Grenzen; geaendert nur Namensraum, der Name der Faelle
 *       (Vorsatz „InstrumentMeasure 34e“) und der Pfad zu `werte/hoerschwellen.txt` (vom Arbeitsordner aus — die
 *       Test-Exe aus dem Projekt-Root starten).
 * @brief Das Setz-Werkzeug an synthetischen Aufgaben (AUFTRAG_BAU §6): ein Modell erzeugt Ziele aus bekannten Werten,
 *        die Suche findet sie wieder — mit einem wirkungslosen (L9), einem verrauschten (L11) und einem welligen
 *        Parameter (L12); Gruppen unabhaengig (L8); Protokoll und Wiederaufnahme (L7); hoerbar getroffen nach HW2 a
 *        (L13, IC-A2); der Schnitt (L14); zwei Traeger derselben Wahrnehmung nicht allein gestaffelt, Glaette des Ziels
 *        (L16); mehrere Startpunkte, der beste je Gruppe, eindeutig, Glaette nach ZF8 a (2) und die Probe HW3 a (IC-A3)
 */
#include <doctest.h>

#include "InstrumentMeasure/Setzen.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <filesystem>

using namespace lumi;
using namespace lumi::messung::setzen;

namespace
{

// die synthetische „Note“: f0, T60, Pegel, Inharmonizitaet B, Verstimmung δ (wellig), ein Parameter ohne Wirkung und
// einer, dessen Wirkung verrauscht ist
std::vector<double> modell(std::size_t, const std::vector<double>& w)
{
    const double f0 = w[0], t60 = w[1], pegel = w[2], b = w[3], d = w[4], rausch = w[6];
    // ein deterministisches Rauschen aus dem Wert (Hash) — die Spalte wird mit h und h/2 verschieden
    std::uint64_t h = static_cast<std::uint64_t>(rausch * 1e12) * 0x9E3779B97F4A7C15ull;
    h ^= h >> 29;
    const double r = static_cast<double>(h % 1000003) / 1000003.0 - 0.5;
    return {f0, std::log(t60), pegel, 2.0 * std::sqrt((1.0 + 4.0 * b) / (1.0 + b)),
            // wellig in δ: eine Schwebung als Anteil mit Nebenminima (wie L12: 2–4 Minima)
            std::cos(2.0 * 3.141592653589793 * 1.5 * d) * std::exp(-0.3 * d) + 0.4 * d,
            rausch + 1e-3 * r};
}

Gruppe aufgabe(const std::string& name, double f0, double t60, double pegel, double b, double d, bool welligMarkiert)
{
    Gruppe g;
    g.name = name;
    g.parameter = {{"f0", 0.0, 50.0, 2000.0, Skala::Log},  {"t60", 0.0, 0.1, 10.0, Skala::Log}, {"pegel", 0.0, -40.0, 0.0, Skala::Linear},
                   {"B", 0.0, 1e-6, 1e-2, Skala::Log},     {"delta", 0.0, 0.0, 2.0, Skala::Linear, welligMarkiert},
                   {"ohne", 0.5, 0.0, 1.0, Skala::Linear}, {"rausch", 0.5, 0.0, 1.0, Skala::Linear}};
    const std::vector<double> wahr = {f0, t60, pegel, b, d, 0.5, 0.37};
    const std::vector<double> m = modell(0, wahr);
    const char* namen[] = {"f0", "lnT60", "pegel", "f2/f1", "schwebung", "rausch"};
    const double tol[] = {0.5, 0.01, 0.1, 1e-5, 0.002, 1e-3};
    for (std::size_t i = 0; i < m.size(); ++i) g.ziele.push_back({namen[i], m[i], tol[i]});
    // der Start daneben
    g.parameter[0].wert = f0 * 1.3;
    g.parameter[1].wert = t60 * 0.5;
    g.parameter[2].wert = pegel + 8.0;
    g.parameter[3].wert = b * 4.0;
    g.parameter[4].wert = 0.05;  // weit weg vom wahren δ, hinter einem Nebenminimum
    g.parameter[6].wert = 0.5;
    return g;
}

}  // namespace

TEST_CASE("InstrumentMeasure 34e Setzen: findet die Werte wieder; wirkungslos (L9) fest und berichtet, verrauscht (L11) gemeldet, wellig (L12) eindimensional")
{
    std::vector<Gruppe> g = {aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true), aufgabe("C2", 65.4, 6.0, -6.0, 5e-5, 0.9, true)};
    Einstellung e;
    const Bericht b = setze(g, modell, e);
    std::printf("%s", b.text().c_str());
    const double wahr[2][5] = {{440.0, 2.0, -12.0, 2e-4, 1.3}, {65.4, 6.0, -6.0, 5e-5, 0.9}};
    for (std::size_t k = 0; k < 2; ++k)
    {
        CAPTURE(k);
        const GruppenBericht& r = b.gruppen[k];
        for (std::size_t j = 0; j < 5; ++j) CHECK(r.werte[j] == doctest::Approx(wahr[k][j]).epsilon(0.01));
        REQUIRE(r.wirkungslos.size() == 1);
        CHECK(r.wirkungslos.front() == "ohne");
        CHECK(r.werte[5] == 0.5);  // nicht bewegt
        CHECK(std::find(r.verrauscht.begin(), r.verrauscht.end(), "rausch") != r.verrauscht.end());
        CHECK(r.treffer >= 5);
        CHECK(r.rest < r.restAnfang);
        CHECK(r.hoerbarGetroffen == r.treffer);  // ohne Hoerschwellen zaehlt nach HW2 a nur τ
    }
}

TEST_CASE("InstrumentMeasure 34e Setzen: ohne L12 bleibt die wellige Groesse im Nebenminimum haengen")
{
    std::vector<Gruppe> mit = {aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true)};
    std::vector<Gruppe> ohne = {aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, false)};
    const Bericht bm = setze(mit, modell);
    const Bericht bo = setze(ohne, modell);
    std::printf("δ mit L12 %.4f (Rest %.3g), ohne %.4f (Rest %.3g); wahr 1,3\n", bm.gruppen[0].werte[4], bm.gruppen[0].rest, bo.gruppen[0].werte[4],
                bo.gruppen[0].rest);
    CHECK(bm.gruppen[0].werte[4] == doctest::Approx(1.3).epsilon(0.01));
    CHECK(std::abs(bo.gruppen[0].werte[4] - 1.3) > 0.03);
    CHECK(bo.gruppen[0].rest > 2.0 * bm.gruppen[0].rest);  // der Rest enthaelt das Rauschen der Spalte „rausch“
}

TEST_CASE("InstrumentMeasure 34e Setzen: Gruppen sind unabhaengig (L8) — allein dasselbe wie zusammen")
{
    std::vector<Gruppe> allein = {aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true)};
    std::vector<Gruppe> zusammen = {aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true), aufgabe("C2", 65.4, 6.0, -6.0, 5e-5, 0.9, true)};
    const Bericht a = setze(allein, modell);
    const Bericht z = setze(zusammen, modell);
    CHECK(a.gruppen[0].werte == z.gruppen[0].werte);
    CHECK(a.gruppen[0].iterationen == z.gruppen[0].iterationen);
}

TEST_CASE("InstrumentMeasure 34e Setzen: je Iteration eine Datei, die Wiederaufnahme setzt fort und endet gleich (L7)")
{
    const std::string ordner = (std::filesystem::temp_directory_path() / "lumiviz_34e_setzen_test").string();
    std::filesystem::remove_all(ordner);
    std::vector<Gruppe> ganz = {aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true)};
    const Bericht b0 = setze(ganz, modell);
    std::vector<Gruppe> teil = {aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true)};
    Einstellung e;
    e.protokoll = ordner;
    e.iterationenHoechstens = 2;
    setze(teil, modell, e);
    CHECK(std::filesystem::exists(std::filesystem::path(ordner) / "iteration_002.tsv"));
    std::vector<Gruppe> weiter = {aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true)};
    e.iterationenHoechstens = 40;
    const Bericht b1 = setze(weiter, modell, e);
    CHECK(b1.ersteIteration == 2);
    for (std::size_t j = 0; j < 5; ++j) CHECK(b1.gruppen[0].werte[j] == doctest::Approx(b0.gruppen[0].werte[j]).epsilon(1e-6));
    std::filesystem::remove_all(ordner);
}

TEST_CASE("InstrumentMeasure 34e Setzen: der Schnitt je Parameter (L14)")
{
    const Gruppe g = aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true);
    const auto s = schnitt(g, 0, 0, modell, 15);
    REQUIRE(s.size() == 15);
    CHECK(s.front().first == doctest::Approx(50.0));
    CHECK(s.back().first == doctest::Approx(2000.0));
    CHECK(s[7].second[0] == doctest::Approx(s[7].first));
}

TEST_CASE("InstrumentMeasure 34e Setzen: zwei Traeger derselben Wahrnehmung werden nicht allein gestaffelt (L16), die Glaette des Ziels")
{
    // die Schwebung aus zwei Mechanismen: δ (Index 4) und „ohne“ (Index 5, hier als zweiter Traeger markiert)
    Gruppe g = aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true);
    g.parameter[4].traegt = "schwebung";
    g.parameter[5].traegt = "schwebung";
    std::vector<Gruppe> gg = {g};
    const Bericht b = setze(gg, modell);
    // „ohne“ ist wirkungslos (L9) und damit fest: δ teilt mit keinem freien Parameter und wird gestaffelt
    CHECK(b.gruppen[0].nichtGestaffelt.empty());
    CHECK(b.gruppen[0].werte[4] == doctest::Approx(1.3).epsilon(0.01));
    // mit einem freien zweiten Traeger (t60, nur zur Probe markiert) bleibt δ ungestaffelt
    Gruppe h = aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true);
    h.parameter[4].traegt = "schwebung";
    h.parameter[1].traegt = "schwebung";
    std::vector<Gruppe> hh = {h};
    const Bericht c = setze(hh, modell);
    REQUIRE(c.gruppen[0].nichtGestaffelt.size() == 1);
    CHECK(c.gruppen[0].nichtGestaffelt.front() == "delta");
    CHECK(c.text().find("nicht gestaffelt (L16): delta") != std::string::npos);
    CHECK(c.text().rfind("Vorschlag (L18)", 0) == 0);
    // die Glaette des Ziels: „schwebung“ (Index 4) ist in δ wellig, in f0 glatt
    const Gruppe w = aufgabe("A4", 440.0, 2.0, -12.0, 2e-4, 1.3, true);
    CHECK(welligkeit(w, 0, 4, modell)[4] >= 2);
    CHECK(welligkeit(w, 0, 0, modell)[0] <= 1);
}

namespace
{

// IC-A3: eine Gruppe mit einem Parameter x ∈ [0, 4]. „einfach“: zwei lineare Groessen, Σ r² eine Parabel (mit x² stiege
// sie zum Rand wie x⁴, und der letzte Schritt laege ueber dem Dreifachen des mittleren — nach ZF8 a (2) nicht glatt).
// „zweifach“: (x − 1)(x − 3) eng, dazu x weit — zwei Becken, das bei 1 ist das tiefere.
std::vector<double> einfach(std::size_t, const std::vector<double>& w)
{
    return {w[0], 0.5 * w[0] + 1.0};
}

std::vector<double> zweifach(std::size_t, const std::vector<double>& w)
{
    return {(w[0] - 1.0) * (w[0] - 3.0), w[0]};
}

Gruppe eine(const std::string& name, const Modell& m, double wahr, std::vector<double> tol, double start)
{
    Gruppe g;
    g.name = name;
    g.parameter = {{"x", start, 0.0, 4.0, Skala::Linear}};
    const std::vector<double> z = m(0, {wahr});
    g.ziele = {{"a", z[0], tol[0]}, {"b", z[1], tol[1]}};
    return g;
}

}  // namespace

TEST_CASE("InstrumentMeasure 34e Setzen: mehrere Startpunkte, der beste je Gruppe (A5), eindeutig nach HW2 a")
{
    const messung::Hoerschwellen leer;
    // ein Becken: beide Starts enden am selben Ort, alles eindeutig
    {
        const std::vector<Gruppe> g = {eine("ein", einfach, 1.7, {0.01, 0.01}, 0.5)};
        const Mehrfach m = setzeMehrfach(g, {{{0.5}}, {{3.5}}}, einfach);
        REQUIRE(m.laeufe.size() == 2);
        CHECK(m.laeufe[0].gruppen[0].werte[0] == doctest::Approx(1.7).epsilon(1e-3));
        CHECK(m.laeufe[1].gruppen[0].werte[0] == doctest::Approx(1.7).epsilon(1e-3));
        CHECK(eindeutig(m, 0, leer) == std::vector<bool>{true, true});
    }
    // zwei Becken: Start 1 endet bei 3 — der beste ist Start 0, „b“ (= x) ist ueber die Starts nicht eindeutig
    {
        const std::vector<Gruppe> g = {eine("zwei", zweifach, 1.0, {0.01, 0.5}, 0.2)};
        const Mehrfach m = setzeMehrfach(g, {{{0.2}}, {{3.8}}}, zweifach);
        std::printf("zwei Becken: Start 0 bei %.4f (Rest %.3g), Start 1 bei %.4f (Rest %.3g)\n", m.laeufe[0].gruppen[0].werte[0],
                    m.laeufe[0].gruppen[0].rest, m.laeufe[1].gruppen[0].werte[0], m.laeufe[1].gruppen[0].rest);
        CHECK(m.laeufe[0].gruppen[0].werte[0] == doctest::Approx(1.0).epsilon(1e-2));
        CHECK(m.laeufe[1].gruppen[0].werte[0] > 2.5);
        CHECK(m.herkunft == std::vector<std::size_t>{0});
        CHECK(m.beste[0].parameter[0].wert == doctest::Approx(1.0).epsilon(1e-2));
        CHECK(m.bester.gruppen[0].rest == m.laeufe[0].gruppen[0].rest);
        CHECK_FALSE(eindeutig(m, 0, leer)[1]);
    }
}

TEST_CASE("InstrumentMeasure 34e Setzen: Glaette nach ZF8 a (2) — ein Becken glatt, zwei Becken nicht")
{
    const Gruppe a = eine("ein", einfach, 1.7, {0.01, 0.01}, 1.7);
    const Glaette ga = glaette(a, 0, 0, einfach);
    CHECK(ga.minima == 1);
    CHECK(ga.glatt());
    const Gruppe b = eine("zwei", zweifach, 1.0, {0.01, 0.5}, 1.0);
    const Glaette gb = glaette(b, 0, 0, zweifach);
    CHECK(gb.minima == 2);
    CHECK_FALSE(gb.glatt());
}

TEST_CASE("InstrumentMeasure 34e Setzen: die Probe nach HW3 a — besteht mit einem Becken, verfehlt an (2) mit zweien, nicht erreichbar zaehlt nicht")
{
    const messung::Hoerschwellen leer;
    {
        const std::vector<Gruppe> g = {eine("ein", einfach, 1.7, {0.01, 0.01}, 0.5)};
        const Mehrfach m = setzeMehrfach(g, {{{0.5}}, {{3.5}}}, einfach);
        const Probe p = probe(g, m, einfach, leer);
        std::printf("%s", p.text().c_str());
        CHECK(p.glatt == 1);
        CHECK(p.eindeutig == 2);
        CHECK(p.messbar == 2);
        CHECK(p.hoerbarVorgabe == 0);
        CHECK(p.hoerbarBest == 2);
        CHECK(p.bestanden());
    }
    {
        const std::vector<Gruppe> g = {eine("zwei", zweifach, 1.0, {0.01, 0.5}, 0.2)};
        const Mehrfach m = setzeMehrfach(g, {{{0.2}}, {{3.8}}}, zweifach);
        const Probe p = probe(g, m, zweifach, leer);
        CHECK_FALSE(p.glaetteHaelt());
        CHECK(p.nichtGlatt == std::vector<std::string>{"zwei"});
        CHECK_FALSE(p.bestanden());
    }
    {
        // das Ziel liegt jenseits der belegten Grenze 4: die Gruppe endet am Rand und zaehlt bei (3) nicht
        std::vector<Gruppe> g = {eine("rand", einfach, 4.5, {0.01, 0.01}, 1.0)};
        g[0].parameter[0].grenzeBelegt = true;
        const Mehrfach m = setzeMehrfach(g, {{{1.0}}, {{2.0}}}, einfach);
        const Probe p = probe(g, m, einfach, leer);
        CHECK(p.nichtErreichbar == std::vector<std::string>{"rand"});
        CHECK(p.messbar == 0);
        CHECK_FALSE(p.eindeutigGenug());
    }
}

TEST_CASE("InstrumentMeasure 34e Setzen: hoerbar getroffen mit der Tabelle (HW1 a, HW2 a) — 1,3-mal so lang ist nicht in τ, aber unter 1,4309")
{
    const messung::HoerschwellenGelesen t = messung::liesHoerschwellen("projects/apps/LumiViz/tests/unit/UnitTests/werte/hoerschwellen.txt");
    REQUIRE(t.werte.has_value());
    const Modell lang = [](std::size_t, const std::vector<double>& w) { return std::vector<double>{w[0]}; };
    std::vector<Gruppe> g(1);
    g[0].name = "t";
    g[0].parameter = {{"t60", 2.6, 0.1, 10.0, Skala::Log}};
    g[0].parameter[0].fest = true;
    g[0].ziele = {{"t60_spaet_1", 2.0, messung::kUntergrenzeFaktor, messung::Skala::Faktor}};
    Einstellung e;
    const Bericht ohne = setze(g, lang, e);
    CHECK(ohne.gruppen[0].treffer == 0);
    CHECK(ohne.gruppen[0].hoerbarGetroffen == 0);
    CHECK(ohne.gruppen[0].groessen[0].residuum == doctest::Approx(std::log(1.3) / messung::kUntergrenzeFaktor));
    e.hoerschwellen = &*t.werte;
    const Bericht mit = setze(g, lang, e);
    CHECK(mit.gruppen[0].treffer == 0);
    CHECK(mit.gruppen[0].hoerbarGetroffen == 1);
    CHECK(alsTasten(mit).front().groessen.size() == 1);
}
