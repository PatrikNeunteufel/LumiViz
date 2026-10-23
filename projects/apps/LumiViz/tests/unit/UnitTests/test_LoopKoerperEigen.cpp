/**
 ****************************************************************************************
 * @file   test_LoopKoerperEigen.cpp
 * @brief  Loop, Stufe 31c-4: der Eigenloeser des Instrumentkoerpers
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S147/S148: BM5-1 a, EG1 a,
 *         EG2 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Gegen die Schnittstelle (`lumi::koerper::Eigenloeser`), nicht gegen Spectra: `eigenloeser()`
 * aus `Umsetzungen.hpp` liefert jede Umsetzung, die es gibt — kommt ein eigener Loeser dazu,
 * laufen dieselben Faelle fuer ihn (BM5-1 a, Zusatz). Ohne Ton, ohne Engine; nur der Steinway liest seinen Bauplan.
 *
 * - Eine Kette aus linearen Elementen (K, M dreibandig) mit geschlossener Loesung: jede Mode im
 *   Fenster, keine doppelt, aufsteigend, bezueglich M orthonormal, kleiner Rest; von unten und
 *   ein Fenster mitten im Spektrum.
 * - Zwei gleiche Ketten ohne Kopplung: jeder Eigenwert doppelt — beide muessen kommen.
 * - Eine frei aufliegende Pruefplatte (k = 3): gegen den dichten Loeser von Eigen (nur im Test)
 *   und gegen Navier; ein Fenster mitten im Spektrum.
 * - Bezugswerte der Pruefplatte (Frequenzen, Form an Stuetzpunkten) in
 *   `werte/eigen_pruefplatte.txt` — neu mit `LUMIVIZ_EIGEN_WERTE_SCHREIBEN=1`.
 * - Fehler und Abbruch.
 * - `LUMIVIZ_EIGEN_MESSUNG=<Hz>`: der Steinway D (Netz bis 5 kHz, k = 3) von 0 bis <Hz>; meldet
 *   Zahl der Moden, Zeit, Zerlegungen, Rest und Spitze des Speichers.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Platte.hpp"
#include "InstrumentBody/Umsetzungen.hpp"
#include "loop/KoerperBauplan.hpp"

#include <Eigen/Dense>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <numbers>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define PSAPI_VERSION 2
#include <windows.h>
#include <psapi.h>
#endif

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

/// Jede Umsetzung der Schnittstelle — eine neue kommt in `Umsetzungen.hpp` dazu.
using kp::eigenloeser;

/// K und M einer Kette aus `n` linearen Elementen der Laenge h (eingespannt an beiden Enden):
/// K = tridiag(−1, 2, −1) / h, M = h/6 · tridiag(1, 4, 1). `ketten` gleiche Ketten ohne Kopplung.
std::pair<kp::DuenneMatrix, kp::DuenneMatrix> kette(int n, int ketten = 1)
{
    const double h = 1.0 / (n + 1);
    const int gesamt = n * ketten;
    std::vector<std::vector<int>> zeilen(static_cast<std::size_t>(gesamt));
    for (int c = 0; c < ketten; ++c)
        for (int i = 0; i < n; ++i)
        {
            const int g = c * n + i;
            zeilen[g] = {g};
            if (i + 1 < n) zeilen[g].push_back(g + 1);
        }
    auto kopie = zeilen;
    kp::DuenneMatrix k, m;
    k.setzeMuster(gesamt, zeilen);
    m.setzeMuster(gesamt, kopie);
    for (int c = 0; c < ketten; ++c)
        for (int i = 0; i < n; ++i)
        {
            const int g = c * n + i;
            k.addiere(g, g, 2.0 / h);
            m.addiere(g, g, 4.0 * h / 6.0);
            if (i + 1 < n)
            {
                k.addiere(g, g + 1, -1.0 / h);
                m.addiere(g, g + 1, h / 6.0);
            }
        }
    return {std::move(k), std::move(m)};
}

/// Die geschlossene Loesung der Kette, aufsteigend, in Hz.
std::vector<double> ketteFrequenzen(int n)
{
    const double h = 1.0 / (n + 1);
    std::vector<double> f;
    for (int j = 1; j <= n; ++j)
    {
        const double c = std::cos(j * std::numbers::pi / (n + 1));
        f.push_back(kp::frequenzVon(6.0 / (h * h) * (1.0 - c) / (2.0 + c)));
    }
    return f;
}

struct Lauf
{
    kp::EigenErgebnis erg;
    std::vector<kp::Mode> moden;
};

Lauf loese(const kp::Eigenloeser& l, const kp::DuenneMatrix& k, const kp::DuenneMatrix& m, double von, double bis)
{
    Lauf lauf;
    kp::EigenAuftrag a;
    a.k = &k;
    a.m = &m;
    a.vonHz = von;
    a.bisHz = bis;
    a.jeMode = [&](const kp::Mode& mode) { lauf.moden.push_back(mode); };
    lauf.erg = l.loese(a);
    return lauf;
}

/// Groesste Abweichung von φiᵀMφj gegen δij.
double orthoFehler(const std::vector<kp::Mode>& moden, const kp::DuenneMatrix& m)
{
    double e = 0.0;
    std::vector<std::vector<double>> mphi;
    for (const auto& mode : moden) mphi.push_back(m.mal(mode.form));
    for (std::size_t i = 0; i < moden.size(); ++i)
        for (std::size_t j = 0; j < moden.size(); ++j)
        {
            double s = 0.0;
            for (std::size_t q = 0; q < mphi[j].size(); ++q) s += moden[i].form[q] * mphi[j][q];
            e = std::max(e, std::abs(s - (i == j ? 1.0 : 0.0)));
        }
    return e;
}

/// Was jeder Lauf erfuellen muss: kein Fehler, Zahl wie gezaehlt und wie erwartet, aufsteigend,
/// im Fenster, Formen wie Frequenzen, orthonormal, kleiner Rest.
void pruefeLauf(const Lauf& lauf, const kp::DuenneMatrix& m, double von, double bis, int erwartet)
{
    REQUIRE_MESSAGE(lauf.erg.fehler.empty(), lauf.erg.fehler);
    CHECK(lauf.erg.imFenster == erwartet);
    REQUIRE(static_cast<int>(lauf.erg.frequenzenHz.size()) == erwartet);
    REQUIRE(lauf.moden.size() == lauf.erg.frequenzenHz.size());
    CHECK(std::is_sorted(lauf.erg.frequenzenHz.begin(), lauf.erg.frequenzenHz.end()));
    for (std::size_t i = 0; i < lauf.moden.size(); ++i)
    {
        CHECK(lauf.moden[i].frequenzHz == lauf.erg.frequenzenHz[i]);
        CHECK(lauf.moden[i].frequenzHz >= von);
        CHECK(lauf.moden[i].frequenzHz < bis);
        CHECK(static_cast<int>(lauf.moden[i].form.size()) == m.n);
    }
    CHECK(orthoFehler(lauf.moden, m) < 1e-8);
    CHECK(lauf.erg.groessterRest < 1e-6);
    CHECK(lauf.erg.faktorisierungen > 0);
}

int zahlIn(const std::vector<double>& f, double von, double bis)
{
    return static_cast<int>(std::count_if(f.begin(), f.end(), [&](double x) { return x >= von && x < bis; }));
}

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

/// Die frei aufliegende Pruefplatte aus test_LoopKoerperPlatte: 0,3 × 0,2 m, 8 mm Fichte, k = 3.
kp::PlattenSystem pruefplatte()
{
    kp::NetzAuftrag auftrag;
    auftrag.umriss = {{0.0, 0.0}, {0.3, 0.0}, {0.3, 0.2}, {0.0, 0.2}};
    auftrag.kante = 0.04;
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(auftrag);
    REQUIRE(netz.fehler.empty());
    const kp::Material mat = fichte();
    kp::PlattenSystem s = kp::baueSystem(netz, kp::LagrangeDreieck(3),
                                         [&](kp::Punkt2) {
                                             kp::Steife st = kp::gedreht(mat, 0.0);
                                             st.h = 0.008;
                                             return st;
                                         },
                                         kp::Auflager::GelenkigAchsparallel);
    REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
    return s;
}

/// Alle Eigenfrequenzen dicht (nur fuer kleine Systeme).
std::vector<double> dichtAlle(const kp::DuenneMatrix& ks, const kp::DuenneMatrix& ms)
{
    const int n = ks.n;
    Eigen::MatrixXd k = Eigen::MatrixXd::Zero(n, n), m = Eigen::MatrixXd::Zero(n, n);
    for (int i = 0; i < n; ++i)
        for (std::int64_t e = ks.start[i]; e < ks.start[i + 1]; ++e)
        {
            const int j = ks.spalten[static_cast<std::size_t>(e)];
            k(i, j) = k(j, i) = ks.werte[static_cast<std::size_t>(e)];
            m(i, j) = m(j, i) = ms.werte[static_cast<std::size_t>(e)];
        }
    Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::MatrixXd> l(k, m, Eigen::EigenvaluesOnly);
    std::vector<double> f;
    for (int i = 0; i < n; ++i) f.push_back(kp::frequenzVon(l.eigenvalues()(i)));
    return f;
}

/// Eine Grenze mitten zwischen zwei Moden.
double zwischen(const std::vector<double>& f, std::size_t i)
{
    return (f[i] + f[i + 1]) / 2.0;
}

/// Die Freiheitsgrade w an den Knoten, die den Stuetzpunkten am naechsten liegen.
std::vector<int> stuetzpunkte(const kp::PlattenSystem& s, const std::vector<kp::Punkt2>& punkte)
{
    std::vector<int> aus;
    for (const auto& p : punkte)
    {
        int best = -1;
        double d = 1e300;
        for (std::size_t q = 0; q < s.knoten.size(); ++q)
        {
            const double dq = std::hypot(s.knoten[q].x - p.x, s.knoten[q].y - p.y);
            if (dq < d && s.frei[3 * q] >= 0)
            {
                d = dq;
                best = s.frei[3 * q];
            }
        }
        aus.push_back(best);
    }
    return aus;
}

QString werteDatei()
{
    return QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir().filePath(QStringLiteral("werte/eigen_pruefplatte.txt"));
}

double spitzeMB()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) return static_cast<double>(pmc.PeakWorkingSetSize) / 1048576.0;
#endif
    return -1.0;
}

}  // namespace

TEST_CASE("Loop Koerper 31c-4 Eigen Kette")
{
    const int n = 1500;
    const auto [k, m] = kette(n);
    const auto soll = ketteFrequenzen(n);
    for (const auto& l : eigenloeser())
    {
        CAPTURE(l->name());
        // von unten bis zwischen Mode 200 und 201
        {
            const double bis = zwischen(soll, 199);
            const Lauf lauf = loese(*l, k, m, 0.0, bis);
            pruefeLauf(lauf, m, 0.0, bis, 200);
            for (std::size_t i = 0; i < lauf.erg.frequenzenHz.size(); ++i)
                CHECK(lauf.erg.frequenzenHz[i] == doctest::Approx(soll[i]).epsilon(1e-9));
            MESSAGE(l->name(), ": Kette ", n, ", 200 Moden von unten, ", lauf.erg.faktorisierungen, " Zerlegungen, Rest ",
                    lauf.erg.groessterRest, ", ", lauf.erg.sekunden, " s");
        }
        // mitten im Spektrum: Mode 701 bis 950
        {
            const double von = zwischen(soll, 699), bis = zwischen(soll, 949);
            const Lauf lauf = loese(*l, k, m, von, bis);
            pruefeLauf(lauf, m, von, bis, 250);
            for (std::size_t i = 0; i < lauf.erg.frequenzenHz.size(); ++i)
                CHECK(lauf.erg.frequenzenHz[i] == doctest::Approx(soll[700 + i]).epsilon(1e-9));
        }
    }
}

TEST_CASE("Loop Koerper 31c-4 Eigen doppelt")
{
    // zwei gleiche Ketten: jeder Eigenwert zweimal — eine Grenze zwischen den Paaren, beide kommen
    const int n = 600;
    const auto [k, m] = kette(n, 2);
    const auto soll = ketteFrequenzen(n);
    for (const auto& l : eigenloeser())
    {
        CAPTURE(l->name());
        const double bis = zwischen(soll, 89);
        const Lauf lauf = loese(*l, k, m, 0.0, bis);
        pruefeLauf(lauf, m, 0.0, bis, 180);
        for (std::size_t i = 0; i < lauf.erg.frequenzenHz.size(); ++i)
            CHECK(lauf.erg.frequenzenHz[i] == doctest::Approx(soll[i / 2]).epsilon(1e-9));
    }
}

TEST_CASE("Loop Koerper 31c-4 Eigen Pruefplatte")
{
    const kp::PlattenSystem s = pruefplatte();
    const auto dicht = dichtAlle(s.k, s.m);
    for (const auto& l : eigenloeser())
    {
        CAPTURE(l->name());
        // die ersten 60 Moden
        const double bis = zwischen(dicht, 59);
        const Lauf lauf = loese(*l, s.k, s.m, 0.0, bis);
        pruefeLauf(lauf, s.m, 0.0, bis, zahlIn(dicht, 0.0, bis));
        for (std::size_t i = 0; i < lauf.erg.frequenzenHz.size(); ++i)
            CHECK(lauf.erg.frequenzenHz[i] == doctest::Approx(dicht[i]).epsilon(1e-8));
        MESSAGE(l->name(), ": Pruefplatte ", s.k.n, " Groessen, ", lauf.erg.frequenzenHz.size(), " Moden bis ", bis, " Hz, ",
                lauf.erg.faktorisierungen, " Zerlegungen, Rest ", lauf.erg.groessterRest, ", ", lauf.erg.sekunden, " s");
        // mitten im Spektrum: Mode 41 bis 120
        const double von2 = zwischen(dicht, 39), bis2 = zwischen(dicht, 119);
        const Lauf mitte = loese(*l, s.k, s.m, von2, bis2);
        pruefeLauf(mitte, s.m, von2, bis2, zahlIn(dicht, von2, bis2));
        for (std::size_t i = 0; i < mitte.erg.frequenzenHz.size(); ++i)
            CHECK(mitte.erg.frequenzenHz[i] == doctest::Approx(dicht[40 + i]).epsilon(1e-8));
    }
}

TEST_CASE("Loop Koerper 31c-4 Eigen Bezugswerte")
{
    // der Bezug fuer den Vergleich alt gegen neu (BM5-1 a): Frequenzen und Form an Stuetzpunkten
    const kp::PlattenSystem s = pruefplatte();
    const auto stuetzen = stuetzpunkte(s, {{0.07, 0.05}, {0.15, 0.1}, {0.21, 0.06}, {0.11, 0.15}, {0.26, 0.17}});
    const int moden = 20;
    for (const auto& l : eigenloeser())
    {
        CAPTURE(l->name());
        kp::EigenAuftrag a;
        a.k = &s.k;
        a.m = &s.m;
        a.bisHz = 4000.0;
        std::vector<std::vector<double>> formen;
        a.jeMode = [&](const kp::Mode& mode) {
            if (static_cast<int>(formen.size()) >= moden) return;
            std::vector<double> w;
            for (const int q : stuetzen) w.push_back(mode.form[static_cast<std::size_t>(q)]);
            // Vorzeichen: der betragsgroesste Stuetzwert positiv
            const auto groesster = std::max_element(w.begin(), w.end(), [](double x, double y) { return std::abs(x) < std::abs(y); });
            if (*groesster < 0.0)
                for (double& x : w) x = -x;
            formen.push_back(w);
        };
        const kp::EigenErgebnis erg = l->loese(a);
        REQUIRE_MESSAGE(erg.fehler.empty(), erg.fehler);
        REQUIRE(static_cast<int>(formen.size()) == moden);

        if (qEnvironmentVariableIntValue("LUMIVIZ_EIGEN_WERTE_SCHREIBEN") == 1)
        {
            QFile datei(werteDatei());
            REQUIRE(datei.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
            QTextStream aus(&datei);
            aus.setRealNumberPrecision(14);
            aus << "# Pruefplatte 0,3 x 0,2 m, 8 mm Fichte, frei aufliegend, k = 3 (" << QString::fromStdString(l->name())
                << ") - geschrieben von test_LoopKoerperEigen.cpp (LUMIVIZ_EIGEN_WERTE_SCHREIBEN=1)\n";
            aus << "# je Mode: Frequenz in Hz, dann w an fuenf Stuetzpunkten (M-normiert, der groesste positiv)\n";
            for (int i = 0; i < moden; ++i)
            {
                aus << erg.frequenzenHz[static_cast<std::size_t>(i)];
                for (const double w : formen[static_cast<std::size_t>(i)]) aus << ' ' << w;
                aus << '\n';
            }
            MESSAGE("eigen_pruefplatte.txt neu geschrieben");
            break;  // nur die erste Umsetzung schreibt
        }
        QFile datei(werteDatei());
        REQUIRE_MESSAGE(datei.open(QIODevice::ReadOnly | QIODevice::Text),
                        "werte/eigen_pruefplatte.txt fehlt — mit LUMIVIZ_EIGEN_WERTE_SCHREIBEN=1 schreiben");
        int i = 0;
        while (!datei.atEnd() && i < moden)
        {
            const QString zeile = QString::fromUtf8(datei.readLine()).trimmed();
            if (zeile.startsWith(QLatin1Char('#')) || zeile.isEmpty()) continue;
            const QStringList teile = zeile.split(QLatin1Char(' '));
            REQUIRE(teile.size() == 1 + static_cast<int>(stuetzen.size()));
            CAPTURE(i);
            CHECK(erg.frequenzenHz[static_cast<std::size_t>(i)] == doctest::Approx(teile[0].toDouble()).epsilon(1e-7));
            const auto& w = formen[static_cast<std::size_t>(i)];
            double skala = 0.0;
            for (const double x : w) skala = std::max(skala, std::abs(x));
            for (std::size_t j = 0; j < w.size(); ++j)
                CHECK(w[j] == doctest::Approx(teile[static_cast<int>(j) + 1].toDouble()).epsilon(1e-5).scale(skala));
            ++i;
        }
        CHECK(i == moden);
    }
}

TEST_CASE("Loop Koerper 31c-4 Eigen Fehler und Abbruch")
{
    const auto [k, m] = kette(300);
    for (const auto& l : eigenloeser())
    {
        CAPTURE(l->name());
        kp::EigenAuftrag a;
        a.k = &k;
        a.m = &m;
        a.bisHz = 100.0;
        a.vonHz = 200.0;
        CHECK_FALSE(l->loese(a).fehler.empty());  // leeres Fenster
        a.vonHz = 0.0;
        a.k = nullptr;
        CHECK_FALSE(l->loese(a).fehler.empty());  // ohne K
        // anderes Muster
        auto [k2, m2] = kette(300);
        std::vector<std::vector<int>> zeilen(300);
        for (int i = 0; i < 300; ++i) zeilen[static_cast<std::size_t>(i)] = {i};
        m2.setzeMuster(300, zeilen);
        for (int i = 0; i < 300; ++i) m2.addiere(i, i, 1.0);
        a.k = &k2;
        a.m = &m2;
        CHECK_FALSE(l->loese(a).fehler.empty());
        // abgebrochen
        a.k = &k;
        a.m = &m;
        a.bisHz = ketteFrequenzen(300)[150];
        int gefragt = 0;
        a.weiter = [&](double) { return ++gefragt < 2; };
        const kp::EigenErgebnis erg = l->loese(a);
        CHECK(erg.fehler == "cancelled");
        CHECK(gefragt == 2);
        // nichts im Fenster: kein Fehler, keine Mode
        const auto soll = ketteFrequenzen(300);
        a.weiter = {};
        a.vonHz = soll[10] * 1.0000001;
        a.bisHz = soll[11] * 0.9999999;
        const kp::EigenErgebnis leer = l->loese(a);
        CHECK(leer.fehler.empty());
        CHECK(leer.imFenster == 0);
        CHECK(leer.frequenzenHz.empty());
    }
}

TEST_CASE("Loop Koerper 31c-4 Eigen Messung Steinway D")
{
    const double bisHz = qEnvironmentVariable("LUMIVIZ_EIGEN_MESSUNG").toDouble();
    if (!(bisHz > 0.0))
    {
        MESSAGE("LUMIVIZ_EIGEN_MESSUNG=<Hz> rechnet den Steinway D (Netz bis 5 kHz, k = 3) bis zu dieser Frequenz");
        return;
    }
    kp::Bauplan plan;
    REQUIRE_MESSAGE(ladeBauplan(QStringLiteral("asset/loop/instrument_bodies/steinway_d") + QLatin1String(kBauplanEndung), plan),
                    "die Test-Exe aus dem Projekt-Root starten");
    const kp::Netz netz = kp::NetzgeberCdt().vernetze(kp::netzAuftrag(plan, 5000.0));
    REQUIRE(netz.fehler.empty());
    const kp::PlattenSystem s =
        kp::baueSystem(netz, kp::LagrangeDreieck(3), [&](kp::Punkt2 p) { return kp::steifeBei(plan, p); }, kp::Auflager::Eingespannt);
    REQUIRE_MESSAGE(s.fehler.empty(), s.fehler);
    MESSAGE("Steinway D: ", s.k.n, " Groessen, Spitze vor dem Loeser ", spitzeMB(), " MB");
    for (const auto& l : eigenloeser())
    {
        CAPTURE(l->name());
        kp::EigenAuftrag a;
        a.k = &s.k;
        a.m = &s.m;
        a.bisHz = bisHz;
        std::vector<double> ersteFrequenzen;
        a.jeMode = [&](const kp::Mode& mode) {
            if (ersteFrequenzen.size() < 10) ersteFrequenzen.push_back(mode.frequenzHz);
        };
        const kp::EigenErgebnis erg = l->loese(a);
        REQUIRE_MESSAGE(erg.fehler.empty(), erg.fehler);
        CHECK(static_cast<int>(erg.frequenzenHz.size()) == erg.imFenster);
        std::string erste;
        for (const double f : ersteFrequenzen) erste += std::to_string(f) + " ";
        MESSAGE(l->name(), ": bis ", bisHz, " Hz ", erg.frequenzenHz.size(), " Moden (gezaehlt ", erg.imFenster, "), ", erg.faktorisierungen,
                " Zerlegungen (", erg.sekundenZerlegung, " s), Rest ", erg.groessterRest, ", ", erg.sekunden, " s, Spitze ", spitzeMB(),
                " MB; erste Frequenzen ", erste);
    }
}
