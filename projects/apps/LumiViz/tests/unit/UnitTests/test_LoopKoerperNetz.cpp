/**
 ****************************************************************************************
 * @file   test_LoopKoerperNetz.cpp
 * @brief  Loop, Stufe 31c-2: das Netz des Instrumentkoerpers (docs/audio/Spielweisen_Konzept.md
 *         §4.1, Entscheid Patrik S147: BM4 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. **Die Faelle pruefen die Schnittstelle**
 * (`lumi::koerper::Netzgeber`), nicht CDT: `netzgeber()` liefert jede Umsetzung, die es
 * gibt — kommt ein eigener Generator dazu, laufen dieselben Faelle fuer ihn (BM4 a, Zusatz).
 *
 * - Die Elementgroesse nach MAESSTRO Gl. 1.
 * - Ein Quadrat mit zwei sich kreuzenden Zwangslinien: Flaeche erhalten, jede Zwangslinie als
 *   Kantenfolge, kleinster Winkel und groesste Flaeche eingehalten, alle Dreiecke gegen den
 *   Uhrzeiger und im Umriss, zwei Laeufe gleich.
 * - Ein Auftrag, den es nicht gibt: ein Fehlertext statt eines Netzes.
 * - Der Steinway D bis 5 kHz (vom Arbeitsordner aus): dieselben Pruefungen; spitze Winkel nur
 *   dort, wo zwei Zwangskanten sie einschliessen (eine Ecke der Eingabe). Die Groessen des
 *   Netzes stehen in `werte/netz_steinway.txt` — der Bezug fuer den Vergleich alt gegen neu;
 *   `LUMIVIZ_NETZ_WERTE_SCHREIBEN=1` schreibt sie neu.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Netz.hpp"
#include "InstrumentBody/Umsetzungen.hpp"
#include "loop/KoerperBauplan.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include <QtGlobal>

#include <chrono>
#include <map>
#include <memory>
#include <set>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

/// Jede Umsetzung der Schnittstelle — ein eigener Generator kommt in `Umsetzungen.hpp` dazu.
using kp::netzgeber;

std::set<std::pair<int, int>> zwangskantenMenge(const kp::Netz& n)
{
    std::set<std::pair<int, int>> m;
    for (const auto& k : n.zwangskanten) m.insert({k[0], k[1]});
    return m;
}

/// Liegt der kleinste Winkel des Dreiecks zwischen zwei Zwangskanten? Dann kommt er aus
/// der Eingabe und laesst sich nicht verfeinern.
bool eckeDerEingabe(const kp::Netz& n, const std::array<int, 3>& d, const std::set<std::pair<int, int>>& zwang)
{
    int spitz = 0;
    double kleinster = 1e9;
    for (int k = 0; k < 3; ++k)
    {
        const kp::Punkt2 p = n.knoten[d[k]], a = n.knoten[d[(k + 1) % 3]], b = n.knoten[d[(k + 2) % 3]];
        const double w = std::atan2(std::abs((a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x)),
                                    (a.x - p.x) * (b.x - p.x) + (a.y - p.y) * (b.y - p.y));
        if (w < kleinster)
        {
            kleinster = w;
            spitz = k;
        }
    }
    const auto hat = [&](int u, int v) { return zwang.count({std::min(u, v), std::max(u, v)}) > 0; };
    return hat(d[spitz], d[(spitz + 1) % 3]) && hat(d[spitz], d[(spitz + 2) % 3]);
}

struct Befund
{
    double flaeche = 0.0;
    double kleinsterWinkel = 180.0;      ///< ohne die Ecken der Eingabe
    double groessteFlaeche = 0.0;
    int eckenDerEingabe = 0;
    int zuSpitz = 0;                     ///< spitzer als verlangt und keine Ecke der Eingabe
    int falschHerum = 0;
    int draussen = 0;
    int fehlendeZwangsstuecke = 0;
};

Befund pruefe(const kp::Netz& n, const kp::NetzAuftrag& a)
{
    Befund b;
    const auto zwang = zwangskantenMenge(n);
    for (const auto& d : n.dreiecke)
    {
        const double f = kp::flaeche(n, d);
        b.flaeche += f;
        if (f <= 0.0) ++b.falschHerum;
        b.groessteFlaeche = std::max(b.groessteFlaeche, f);
        const kp::Punkt2 m{(n.knoten[d[0]].x + n.knoten[d[1]].x + n.knoten[d[2]].x) / 3.0,
                           (n.knoten[d[0]].y + n.knoten[d[1]].y + n.knoten[d[2]].y) / 3.0};
        if (!kp::innen(a.umriss, m)) ++b.draussen;
        const double w = kp::kleinsterWinkelGrad(n, d);
        if (eckeDerEingabe(n, d, zwang))
        {
            ++b.eckenDerEingabe;
            continue;
        }
        b.kleinsterWinkel = std::min(b.kleinsterWinkel, w);
        if (w < a.kleinsterWinkelGrad - 1e-6) ++b.zuSpitz;
    }
    for (const auto& linie : a.zwangslinien)
        for (std::size_t k = 0; k + 1 < linie.size(); ++k)
            if (!kp::aufZwangskanten(n, linie[k], linie[k + 1], 1e-7)) ++b.fehlendeZwangsstuecke;
    for (std::size_t k = 0; k < a.umriss.size(); ++k)
        if (!kp::aufZwangskanten(n, a.umriss[k], a.umriss[(k + 1) % a.umriss.size()], 1e-7)) ++b.fehlendeZwangsstuecke;
    return b;
}

QString werteDatei()
{
    return QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir().filePath(QStringLiteral("werte/netz_steinway.txt"));
}

}  // namespace

TEST_CASE("Loop Koerper 31c-2 Elementgroesse")
{
    kp::Material fichte;
    fichte.dichte = 380.0;
    fichte.ex = 11.0e9;
    fichte.ey = 0.65e9;
    fichte.gxy = 0.66e9;
    fichte.gxz = 1.2e9;
    fichte.gyz = 0.042e9;
    fichte.nuxy = 0.26;
    // λ = √(2π/f)·(D/(ρh))^¼, D = E_y h³/12 — 8 mm bei 5 kHz, von Hand gerechnet
    CHECK(kp::kuerzesteBiegewelle(fichte, 0.008, 5000.0) == doctest::Approx(0.0616080).epsilon(1e-5));
    // λ ∝ 1/√f und ∝ √h
    CHECK(kp::kuerzesteBiegewelle(fichte, 0.008, 20000.0) == doctest::Approx(0.0616080 / 2.0).epsilon(1e-5));
    CHECK(kp::kuerzesteBiegewelle(fichte, 0.032, 5000.0) == doctest::Approx(0.0616080 * 2.0).epsilon(1e-5));

    kp::Bauplan plan;
    plan.materialien = {fichte};
    plan.materialien[0].name = "spruce";
    plan.platte.material = "spruce";
    plan.platte.dicke = 0.008;
    plan.platte.dickeRaster.nx = 1;
    plan.platte.dickeRaster.ny = 1;
    plan.platte.dickeRaster.schritt = 0.1;
    plan.platte.dickeRaster.werte = {0.006};  // die duennste Stelle zaehlt
    CHECK(kp::elementKante(plan, 5000.0) == doctest::Approx(kp::kuerzesteBiegewelle(fichte, 0.006, 5000.0) / 4.0));
    CHECK(kp::elementKante(plan, 0.0) == 0.0);
}

TEST_CASE("Loop Koerper 31c-2 Netz Quadrat")
{
    kp::NetzAuftrag a;
    a.umriss = {{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
    a.zwangslinien = {{{0.1, 0.1}, {0.5, 0.45}, {0.9, 0.9}}, {{0.1, 0.9}, {0.9, 0.1}}};
    a.kante = 0.1;
    for (const auto& geber : netzgeber())
    {
        CAPTURE(geber->name());
        const kp::Netz n = geber->vernetze(a);
        REQUIRE_MESSAGE(n.fehler.empty(), n.fehler);
        const Befund b = pruefe(n, a);
        CHECK(b.flaeche == doctest::Approx(1.0).epsilon(1e-12));
        CHECK(b.falschHerum == 0);
        CHECK(b.draussen == 0);
        CHECK(b.fehlendeZwangsstuecke == 0);
        CHECK(b.zuSpitz == 0);
        CHECK(b.groessteFlaeche <= kp::groessteFlaeche(a.kante) * (1.0 + 1e-9));
        CHECK(n.dreiecke.size() >= static_cast<std::size_t>(1.0 / kp::groessteFlaeche(a.kante)));
        MESSAGE(geber->name(), ": ", n.knoten.size(), " Knoten, ", n.dreiecke.size(), " Dreiecke, kleinster Winkel ",
                b.kleinsterWinkel, " Grad, Ecken der Eingabe ", b.eckenDerEingabe);
        // zwei Laeufe, dasselbe Netz
        CHECK(geber->vernetze(a) == n);
    }
}

TEST_CASE("Loop Koerper 31c-2 Netz Fehler")
{
    for (const auto& geber : netzgeber())
    {
        CAPTURE(geber->name());
        kp::NetzAuftrag a;
        a.umriss = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}};
        a.kante = 0.0;
        const kp::Netz n = geber->vernetze(a);
        CHECK_FALSE(n.fehler.empty());
        CHECK(n.dreiecke.empty());
    }
}

TEST_CASE("Loop Koerper 31c-2 Netz Steinway D")
{
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(QStringLiteral("asset/loop/instrument_bodies/steinway_d") + QLatin1String(kBauplanEndung), plan),
                    "die Test-Exe aus dem Projekt-Root starten");
    const kp::NetzAuftrag a = kp::netzAuftrag(plan, 5000.0);
    CHECK(a.kante == doctest::Approx(kp::elementKante(plan, 5000.0)));
    // 16 Rippen und ein Steg, je zwei Raender — eine Rippe kann am Umriss in Stuecke fallen
    CHECK(a.zwangslinien.size() >= 34);

    for (const auto& geber : netzgeber())
    {
        CAPTURE(geber->name());
        const auto start = std::chrono::steady_clock::now();
        const kp::Netz n = geber->vernetze(a);
        const double sekunden = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        REQUIRE_MESSAGE(n.fehler.empty(), n.fehler);
        const Befund b = pruefe(n, a);
        MESSAGE(geber->name(), ": Kante ", a.kante * 1e3, " mm, ", n.knoten.size(), " Knoten, ", n.dreiecke.size(),
                " Dreiecke, kleinster Winkel ", b.kleinsterWinkel, " Grad, Ecken der Eingabe ", b.eckenDerEingabe,
                ", unverfeinert ", n.unverfeinert, ", ", sekunden, " s");
        CHECK(b.flaeche == doctest::Approx(kp::flaeche(plan.platte.umriss)).epsilon(1e-9));
        CHECK(b.falschHerum == 0);
        CHECK(b.draussen == 0);
        CHECK(b.fehlendeZwangsstuecke == 0);
        CHECK(b.zuSpitz == 0);
        CHECK(b.groessteFlaeche <= kp::groessteFlaeche(a.kante) * (1.0 + 1e-9));

        // der Bezug fuer den Vergleich alt gegen neu (BM4 a): die Groessen des Netzes von CDT
        if (geber->name().rfind("CDT", 0) != 0) continue;
        if (qEnvironmentVariableIntValue("LUMIVIZ_NETZ_WERTE_SCHREIBEN") == 1)
        {
            QFile datei(werteDatei());
            REQUIRE(datei.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
            QTextStream aus(&datei);
            aus.setRealNumberPrecision(12);  // sonst nur sechs Stellen
            aus << "# Netz des Steinway D bis 5 kHz (" << QString::fromStdString(geber->name())
                << ") - geschrieben von test_LoopKoerperNetz.cpp (LUMIVIZ_NETZ_WERTE_SCHREIBEN=1)\n";
            aus << "kante " << a.kante << "\nknoten " << n.knoten.size() << "\ndreiecke " << n.dreiecke.size()
                << "\nkleinster_winkel " << b.kleinsterWinkel << "\necken_der_eingabe " << b.eckenDerEingabe << "\n";
            MESSAGE("netz_steinway.txt neu geschrieben");
            continue;
        }
        QFile datei(werteDatei());
        REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly | QIODevice::Text), "werte/netz_steinway.txt fehlt — mit LUMIVIZ_NETZ_WERTE_SCHREIBEN=1 schreiben");
        std::map<QString, double> bezug;
        while (!datei.atEnd())
        {
            const QString zeile = QString::fromUtf8(datei.readLine()).trimmed();
            if (zeile.startsWith(QLatin1Char('#')) || zeile.isEmpty()) continue;
            const QStringList teile = zeile.split(QLatin1Char(' '));
            if (teile.size() == 2) bezug[teile[0]] = teile[1].toDouble();
        }
        // ein anderer Compiler darf um wenige Prozent abweichen (Rundung in der Verfeinerung)
        CHECK(a.kante == doctest::Approx(bezug["kante"]).epsilon(1e-9));
        CHECK(static_cast<double>(n.knoten.size()) == doctest::Approx(bezug["knoten"]).epsilon(0.02));
        CHECK(static_cast<double>(n.dreiecke.size()) == doctest::Approx(bezug["dreiecke"]).epsilon(0.02));
        CHECK(b.kleinsterWinkel >= bezug["kleinster_winkel"] - 1.0);
    }
}
