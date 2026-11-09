/**
 * @file test_hammer_bezug.cpp
 * @brief Pruefstein des Grundgeruests (AUFTRAG_BAU §3): der portierte Kern der Hammer-Saite gegen den Bezug des
 *        Vorbilds (`werte/hammer_bezug_saite.json`, `werte/hammer_bezug_clang.txt`, LumiViz P8 a)
 *
 * Der Bezug stammt aus LumiViz S153 (P8 a, „Bezug vor dem Umbau“, unter clang): neun Toene (Taste, Stufe,
 * Haltedauer in ms), je Ton die Zahl der Abtastwerte, eine Pruefsumme und der Pegel je 0,5 s. Die Saite der Datei
 * kennt die Hysterese des Filzes noch nicht (HU3-1 c kam in S159) — sie wird mit ε = 0 gerechnet, wie das Programm
 * damals rechnete; die zweite Ebene und der Stoss fehlen ebenso und bekommen die Vorgabe 0.
 *
 * Geprueft wird (P8 a): bei einer Saite der Pegel je Block auf 10⁻⁵ dB (die Datei hat sechs Nachkommastellen), im Chor
 * mit Toleranz (P8 a: „im Chor mit Toleranz (P5 a)“ — der Eigenloeser kam nach dem Bezug, 31d-7) auf 0,01 dB; die
 * Zahl der Abtastwerte genau. **Nicht geprueft:** die Pruefsumme — ihr Verfahren steht nicht in der Kopie des
 * Vorbilds (der Test `test_LoopHammerTon.cpp` wurde nicht kopiert); FNV-1a (64 Bit, ueber Bytes und Worte),
 * `std::hash` und ein Polynom 31 treffen sie nicht (IC Session 4). Offen in `klassen/Klasse 1 - Hammer-Saite/offen.md`.
 */
#include "doctest.h"

#include "gemeinsam/Werkzeug.hpp"
#include "klassen/01_hammer_saite/HammerTon.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace ic;
using namespace ic::hammer;

namespace
{

struct BezugTon
{
    int taste = 0;
    int stufe = 0;
    int haltenMs = 0;
    long long frames = 0;
    std::vector<double> db;
};

std::vector<BezugTon> leseBezug(const std::string& pfad)
{
    std::vector<BezugTon> toene;
    std::ifstream ein(pfad);
    std::string zeile;
    while (std::getline(ein, zeile))
    {
        if (zeile.empty() || zeile[0] == '#') continue;
        std::istringstream z(zeile);
        std::string name, frames, pruef, db;
        std::getline(z, name, '\t');
        std::getline(z, frames, '\t');
        std::getline(z, pruef, '\t');
        std::getline(z, db, '\t');
        BezugTon t;
        std::sscanf(name.c_str(), "m%d:a%d:d%d", &t.taste, &t.stufe, &t.haltenMs);
        t.frames = std::stoll(frames);
        std::istringstream d(db);
        std::string w;
        while (std::getline(d, w, ',')) t.db.push_back(std::stod(w));
        toene.push_back(t);
    }
    return toene;
}

HammerSaite bezugSaite()
{
    JsonWert j;
    std::string fehler;
    REQUIRE(leseJsonDatei(std::string(IC_WERTE) + "/hammer_bezug_saite.json", j, &fehler));
    HammerSaite s = hammerSaiteAusJson(j["hammerString"]);
    if (!j["hammerString"].hat("hammerHysteresis")) s.hammerHysterese = ic::kurveAusPunkten({{21, 0.0}, {108, 0.0}});
    richteHammerSaite(s);
    return s;
}

}  // namespace

TEST_CASE("Hammer-Saite: der Port trifft den Bezug des Vorbilds (P8 a)")
{
    const HammerSaite s = bezugSaite();
    const std::vector<BezugTon> bezug = leseBezug(std::string(IC_WERTE) + "/hammer_bezug_clang.txt");
    REQUIRE(bezug.size() == 9);
    for (const BezugTon& b : bezug)
    {
        HammerAnschlag a;
        a.taste = b.taste;
        a.hz = 440.0 * std::exp2((b.taste - 69) / 12.0);
        a.stufe = b.stufe;
        a.rate = 44100;
        a.halten = std::llround(b.haltenMs * 1e-3 * a.rate);
        a.gedaempft = b.taste < kUngedaempftAbVorgabe;
        const auto frames = a.halten + hammerNachklang(s, a, 1LL << 40);
        CAPTURE(b.taste);
        CAPTURE(b.stufe);
        CAPTURE(b.haltenMs);
        CHECK(frames == b.frames);
        std::vector<float> mono;
        rechneHammerTon(s, a, mono, frames);
        const std::vector<double> db = dbJeBlock(mono, a.rate, 0.5);
        REQUIRE(db.size() == b.db.size());
        const bool chor = chorAn(s, b.taste) > 1;
        // unter −100 dB liegt der letzte, ausgeblendete Block im Rauschen der Ausblendung — dort reicht 0,1 dB
        for (std::size_t i = 0; i < db.size(); ++i)
        {
            CAPTURE(i);
            const double grenze = b.db[i] < -100.0 ? 0.1 : (chor ? 0.01 : 1e-5);
            CHECK(std::abs(db[i] - b.db[i]) <= grenze);
            if (std::getenv("IC_BERICHT")) std::printf("m%d a%d d%d Block %zu: %.6f gegen %.6f (%.2e dB)\n", b.taste, b.stufe, b.haltenMs, i, db[i], b.db[i], db[i] - b.db[i]);
        }
    }
}
