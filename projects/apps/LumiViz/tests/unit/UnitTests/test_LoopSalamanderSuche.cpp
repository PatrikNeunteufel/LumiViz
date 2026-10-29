/**
 ****************************************************************************************
 * @file   test_LoopSalamanderSuche.cpp
 * @brief  Loop, Stufe 31d-7 B, 7.5.10: Levenberg–Marquardt ueber die Stuetzpunkte der Hammer-Saite
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S156/S157/S161: ZF6 a, ZF9 a, SP2 a, SP3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Werkzeuge (ohne die Variable meldet der Fall das und prueft nichts):
 * - `LUMIVIZ_75_SUCHE=<Ordner>` sucht an allen 30 Tasten von beiden Startpunkten aus (`LUMIVIZ_75_SUCHE_START=1|2` nur
 *   einen); je Iteration `iteration_<start>_<n>.txt`, dazu `fortschritt.txt`; ein neuer Lauf mit demselben Ordner nimmt
 *   die letzte Iteration auf. Am Ende `ergebnis_<start>.txt` und, wenn beide Startpunkte fertig sind, `vergleich.txt`
 *   und `kurzfassung.txt`. Laeuft Stunden — nur nach Ruecksprache mit Patrik.
 * - `LUMIVIZ_75_NM=<Ordner>` vergleicht Nelder–Mead (hoechstens 150 Auswertungen) mit Levenberg–Marquardt an den Tasten
 *   33, 60, 84 je allein (nur die Stuetzpunkte dieser Taste frei) und legt `nm.txt` ab.
 * - `LUMIVIZ_75_JACOBI=1` vergleicht im Fall „Jacobi je Kurve“ die Ableitungen am Ton an 33 und 60.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "LevenbergMarquardt.hpp"
#include "Modellton.hpp"
#include "NelderMead.hpp"
#include "Strukturmessung.hpp"
#include "Suche.hpp"
#include "Suchkurven.hpp"
#include "Zielsumme.hpp"

#include "InstrumentBody/Statistik.hpp"
#include "loop/HammerSaite.hpp"

#include <QDateTime>
#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iterator>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::messung;

namespace
{

const std::filesystem::path kStrukturDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_struktur.txt";
const std::filesystem::path kZielDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_ziel.txt";

const char* compilerName()
{
#if defined(__clang__)
    return "clang";
#elif defined(_MSC_VER)
    return "msvc";
#else
    return "anderer";
#endif
}

double sekundenSeit(std::chrono::steady_clock::time_point t)
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t).count();
}

/// Die Aufgabe an den `tasten` um die Vorgabe, mit Struktur und Ziel aus den mitgelieferten Dateien.
modelltest::SuchAufgabe aufgabe(const std::vector<int>& tasten)
{
    const std::optional<Strukturen> strukturen = liesStruktur(kStrukturDatei);
    const std::optional<Ziele> ziele = liesZiele(kZielDatei);
    REQUIRE(strukturen.has_value());
    REQUIRE(ziele.has_value());
    modelltest::SuchAufgabe a;
    a.tasten = tasten;
    a.strukturen = *strukturen;
    a.ziele = *ziele;
    a.raum = modelltest::suchraum(lumi::loop::hammerSaiteVorgabe(), tasten, a.strukturen);
    return a;
}

/// Die Residuen vieler Punkte am Ton (zaehlt die Auswertungen).
LmResiduenViele vieleAmTon(const modelltest::SuchAufgabe& a, int& auswertungen)
{
    return [&a, &auswertungen](const std::vector<std::vector<double>>& punkte) {
        std::vector<std::vector<double>> rs;
        for (const modelltest::Bewertet& b : modelltest::bewerte(a, punkte)) rs.push_back(b.r);
        auswertungen += static_cast<int>(punkte.size());
        return rs;
    };
}

/// Eine Suche am Ton mit der Jacobi-Matrix je Kurve.
struct TonSuche
{
    LmErgebnis ergebnis;
    std::vector<TastenSumme> tasten;  ///< am Ende
    int auswertungen = 0;             ///< alle, mit denen der Jacobi-Matrix
    double sekunden = 0.0;
};

using TonBeobachter = std::function<void(const LmIteration&, const std::vector<TastenSumme>&, double dauerS)>;

TonSuche sucheAmTon(const modelltest::SuchAufgabe& a, const LmStart& start, const LmEinstellungen& einst, const TonBeobachter& beobachter = {})
{
    TonSuche s;
    const auto anfang = std::chrono::steady_clock::now();
    auto letzte = anfang;
    const LmRaum raum = modelltest::lmRaum(a.raum);
    const auto gruppen = modelltest::gruppenJeKurve(a.raum);
    const auto zeilen = modelltest::zeilenJeParameter(a);
    std::map<std::vector<double>, std::vector<TastenSumme>> gemerkt;  // die Summen je ausgewertetem Punkt
    const LmResiduen residuen = [&](const std::vector<double>& x) {
        const std::vector<modelltest::Bewertet> b = modelltest::bewerte(a, {x});
        ++s.auswertungen;
        gemerkt[x] = b.front().tasten;
        return b.front().r;
    };
    const LmResiduenViele viele = vieleAmTon(a, s.auswertungen);
    const LmJacobi jacobi = [&](const std::vector<double>& x, const std::vector<double>& r) {
        return jacobiJeGruppe(viele, x, r, gruppen, differenzSchritte(raum, x, raum.ende), zeilen);
    };
    const LmBeobachter jeIteration = [&](const LmIteration& it) {
        const std::vector<TastenSumme> tasten = gemerkt.count(it.x) ? gemerkt.at(it.x) : std::vector<TastenSumme>{};
        if (beobachter) beobachter(it, tasten, sekundenSeit(letzte));
        letzte = std::chrono::steady_clock::now();
        for (auto i = gemerkt.begin(); i != gemerkt.end();) i = i->first == it.x ? std::next(i) : gemerkt.erase(i);
    };
    s.ergebnis = levenbergMarquardt(residuen, jacobi, start, raum, einst, jeIteration);
    if (gemerkt.count(s.ergebnis.x)) s.tasten = gemerkt.at(s.ergebnis.x);
    s.sekunden = sekundenSeit(anfang);
    return s;
}

std::string zahl(double v, int stellen = 3)
{
    std::ostringstream s;
    s << std::fixed << std::setprecision(stellen) << v;
    return s.str();
}

}  // namespace

TEST_CASE("Loop Salamander 7.5.10 Suche: Levenberg-Marquardt an Aufgaben von Hand")
{
    // ein Abklingen y = a·exp(−t/b) an 51 Punkten mit fest gezogenem Rauschen (SplitMix64, σ = 0,01)
    {
        lumi::koerper::Zufall zufall(7);
        std::vector<double> t, y;
        for (int i = 0; i <= 50; ++i)
        {
            t.push_back(0.1 * i);
            y.push_back(2.0 * std::exp(-t.back() / 1.5) + 0.01 * zufall.normal());
        }
        const auto f = [&](const std::vector<double>& x) {
            std::vector<double> r;
            for (std::size_t i = 0; i < t.size(); ++i) r.push_back((x[0] * std::exp(-t[i] / x[1]) - y[i]) / 0.01);
            return r;
        };
        const LmResiduenViele viele = [&](const std::vector<std::vector<double>>& p) {
            std::vector<std::vector<double>> rs;
            for (const auto& x : p) rs.push_back(f(x));
            return rs;
        };
        const LmRaum raum{{0.1, 0.1}, {10.0, 10.0}, {1e-8, 1e-8}};
        const LmJacobi jacobi = [&](const std::vector<double>& x, const std::vector<double>& r) {
            return jacobiEinzeln(viele, x, r, differenzSchritte(raum, x, {1e-7, 1e-7}));
        };
        LmEinstellungen einst;
        einst.iterationenHoechstens = 100;
        const LmErgebnis e = levenbergMarquardt(f, jacobi, LmStart{{1.0, 1.0}}, raum, einst);
        MESSAGE("Abklingen: a " << e.x[0] << ", b " << e.x[1] << ", Rest " << e.rest << " nach " << e.iterationen << " Iterationen (" << e.ende << ")");
        CHECK(e.x[0] == doctest::Approx(2.0).epsilon(0.01));
        CHECK(e.x[1] == doctest::Approx(1.5).epsilon(0.01));
        CHECK(e.anGrenze.empty());
        CHECK(e.rest < 80.0);  // 51 Punkte mit σ = 1 in der Einheit der Residuen
    }
    // Rosenbrock r = (10·(x1 − x0²), 1 − x0) mit dem Minimum (1, 1) ausserhalb der Grenze x0 ≤ 0,5: endet an der
    // Grenze, x1 = x0², und meldet x0 an der Grenze
    {
        const LmResiduen f = [](const std::vector<double>& x) { return std::vector<double>{10.0 * (x[1] - x[0] * x[0]), 1.0 - x[0]}; };
        const LmJacobi jacobi = [](const std::vector<double>& x, const std::vector<double>&) {
            Matrix j(2, 2);
            j(0, 0) = -20.0 * x[0];
            j(0, 1) = 10.0;
            j(1, 0) = -1.0;
            return j;
        };
        const LmRaum raum{{-2.0, -2.0}, {0.5, 2.0}, {1e-9, 1e-9}};
        LmEinstellungen einst;
        einst.iterationenHoechstens = 200;
        const LmErgebnis e = levenbergMarquardt(f, jacobi, LmStart{{-1.2, 1.0}}, raum, einst);
        MESSAGE("Rosenbrock mit Grenze: x " << e.x[0] << ", " << e.x[1] << ", Rest " << e.rest << " nach " << e.iterationen << " Iterationen ("
                                            << e.ende << ")");
        CHECK(e.x[0] == doctest::Approx(0.5));
        CHECK(e.x[1] == doctest::Approx(0.25).epsilon(1e-3));
        CHECK(e.rest == doctest::Approx(0.25).epsilon(1e-3));
        REQUIRE(e.anGrenze.size() == 1);
        CHECK(e.anGrenze.front() == 0);
    }
    // die Loesung der Normalgleichungen: ein System mit Pivot, eine Spalte ohne Wirkung bekommt 0
    {
        Matrix a(3, 3);
        a(0, 1) = 2.0;
        a(1, 0) = 4.0;
        a(1, 1) = 1.0;
        const std::vector<double> x = loese(a, {4.0, 9.0, 0.0});
        CHECK(x[0] == doctest::Approx(1.75));
        CHECK(x[1] == doctest::Approx(2.0));
        CHECK(x[2] == 0.0);
    }
}

TEST_CASE("Loop Salamander 7.5.10 Suche: Marquardt-Daempfung und Ende")
{
    // r = atan(x) ab x = 3: der volle Gauss-Newton-Schritt schiesst ueber das Minimum hinaus (der Rest steigt), erst ein
    // groesseres λ gibt einen besseren Schritt
    const LmResiduen f = [](const std::vector<double>& x) { return std::vector<double>{std::atan(x[0])}; };
    const LmJacobi jacobi = [](const std::vector<double>& x, const std::vector<double>&) {
        Matrix j(1, 1);
        j(0, 0) = 1.0 / (1.0 + x[0] * x[0]);
        return j;
    };
    const LmRaum raum{{-100.0}, {100.0}, {1e-10}};
    std::vector<LmIteration> iterationen;
    const LmErgebnis e = levenbergMarquardt(f, jacobi, LmStart{{3.0}, 1e-3}, raum, {}, [&](const LmIteration& it) { iterationen.push_back(it); });
    REQUIRE_FALSE(iterationen.empty());
    CHECK(iterationen.front().verworfen >= 1);
    CHECK(iterationen.front().angenommen);
    for (const LmIteration& it : iterationen)
    {
        CAPTURE(it.nummer);
        // jeder verworfene Schritt hebt λ um den Faktor, ein angenommener senkt es um ihn
        CHECK(it.lambda == doctest::Approx(it.lambdaVorher * std::pow(10.0, it.verworfen - (it.angenommen ? 1 : 0))));
        if (it.angenommen) CHECK(it.rest < it.restVorher);
        CHECK(it.versuche == it.verworfen + (it.angenommen ? 1 : 0));
    }
    CHECK(e.ende == "Schritte klein");
    CHECK(std::abs(e.x[0]) < 1e-6);
    CHECK(e.iterationen == static_cast<int>(iterationen.size()));
    CHECK(iterationen.back().ende == e.ende);
    MESSAGE("atan: " << e.iterationen << " Iterationen, im ersten " << iterationen.front().verworfen << " Schritte verworfen, x " << e.x[0]);

    // Ende nach der Zahl der Iterationen; ein neuer Start mit Iteration und λ nimmt dort auf
    LmEinstellungen zwei;
    zwei.iterationenHoechstens = 2;
    const LmErgebnis kurz = levenbergMarquardt(f, jacobi, LmStart{{3.0}, 1e-3}, raum, zwei);
    CHECK(kurz.ende == "Iterationen");
    CHECK(kurz.iterationen == 2);
    const LmErgebnis weiter = levenbergMarquardt(f, jacobi, LmStart{kurz.x, kurz.lambda, kurz.iterationen}, raum, {});
    CHECK(weiter.ende == "Schritte klein");
    CHECK(std::abs(weiter.x[0]) < 1e-6);
    CHECK(weiter.iterationen > 2);
}

TEST_CASE("Loop Salamander 7.5.10 Suche: Jacobi je Kurve gegen einzeln")
{
    // synthetisch: drei „Tasten“, zwei „Kurven“ — die Zeilen einer Taste haengen nur an ihren zwei Parametern
    {
        const LmResiduenViele viele = [](const std::vector<std::vector<double>>& p) {
            std::vector<std::vector<double>> rs;
            for (const auto& x : p)
            {
                std::vector<double> r;
                for (std::size_t t = 0; t < 3; ++t)
                {
                    r.push_back(std::sin(x[t]) * x[3 + t] + static_cast<double>(t));
                    r.push_back(std::exp(0.3 * x[t]) - x[3 + t] * x[3 + t]);
                }
                rs.push_back(std::move(r));
            }
            return rs;
        };
        const std::vector<double> x{0.3, -0.7, 1.1, 2.0, 0.5, -1.2};
        const std::vector<double> r = viele({x}).front();
        std::vector<std::vector<std::size_t>> zeilen;
        for (std::size_t j = 0; j < 6; ++j) zeilen.push_back({2 * (j % 3), 2 * (j % 3) + 1});
        const std::vector<double> h(6, 1e-6);
        const Matrix gruppe = jacobiJeGruppe(viele, x, r, {{0, 1, 2}, {3, 4, 5}}, h, zeilen);
        const Matrix einzeln = jacobiEinzeln(viele, x, r, h);
        CHECK(gruppe == einzeln);  // bitgleich
        CHECK(gruppe(0, 0) == doctest::Approx(std::cos(0.3) * 2.0).epsilon(1e-4));
    }

    // die Gruppen und Zeilen der Suche: eine Gruppe je Kurve, die Zeilen eines Stuetzpunkts sind die seiner Taste
    const modelltest::SuchAufgabe a = aufgabe({33, 60});
    const auto gruppen = modelltest::gruppenJeKurve(a.raum);
    CHECK(gruppen.size() == 9);
    const auto zeilen = modelltest::zeilenJeParameter(a);
    REQUIRE(zeilen.size() == a.raum.zahl());
    const std::size_t z33 = a.ziele.at(33).size();
    const std::size_t z60 = a.ziele.at(60).size();
    CHECK(zeilen.front().size() == z33);  // decay an 33
    CHECK(zeilen.front().front() == 0);
    CHECK(zeilen[1].front() == z33);  // decay an 60
    CHECK(zeilen[1].size() == z60);

    if (qEnvironmentVariable("LUMIVIZ_75_JACOBI") != QStringLiteral("1"))
    {
        MESSAGE("LUMIVIZ_75_JACOBI=1 vergleicht die Ableitungen je Kurve mit den einzeln ausgelenkten am Ton an 33 und 60 (rund 1 min)");
        return;
    }
    const auto anfang = std::chrono::steady_clock::now();
    int auswertungen = 0;
    const LmResiduenViele viele = vieleAmTon(a, auswertungen);
    const std::vector<double> x = modelltest::startpunkt(a.raum, 1);
    const std::vector<double> r = viele({x}).front();
    const LmRaum raum = modelltest::lmRaum(a.raum);
    const std::vector<double> h = differenzSchritte(raum, x, raum.ende);
    const Matrix gruppe = jacobiJeGruppe(viele, x, r, gruppen, h, zeilen);
    const Matrix einzeln = jacobiEinzeln(viele, x, r, h);
    double groesst = 0.0, abstand = 0.0;
    int verschieden = 0;
    for (std::size_t i = 0; i < gruppe.werte.size(); ++i)
    {
        groesst = std::max(groesst, std::abs(einzeln.werte[i]));
        abstand = std::max(abstand, std::abs(gruppe.werte[i] - einzeln.werte[i]));
        verschieden += gruppe.werte[i] != einzeln.werte[i] ? 1 : 0;
    }
    MESSAGE("Jacobi am Ton an 33 und 60: " << auswertungen << " Auswertungen in " << sekundenSeit(anfang) << " s; " << verschieden << " von "
                                           << gruppe.werte.size() << " Eintraegen verschieden, groesster Abstand " << abstand << " bei groesstem Eintrag "
                                           << groesst);
    CHECK(abstand <= 1e-9 * std::max(1.0, groesst));
}

TEST_CASE("Loop Salamander 7.5.10 Suche: eine Iteration am Ton an Taste 60")
{
    const modelltest::SuchAufgabe a = aufgabe({60});
    REQUIRE(a.raum.zahl() == 8);  // decay, lateDecay, highDecay, detune, u leise, u laut, gamma, h
    LmEinstellungen einst;
    einst.iterationenHoechstens = 1;
    const LmStart start{modelltest::startpunkt(a.raum, 1), einst.lambdaStart};
    std::optional<LmIteration> iteration;
    std::vector<TastenSumme> summen;
    double dauer = 0.0;
    const TonSuche s = sucheAmTon(a, start, einst, [&](const LmIteration& it, const std::vector<TastenSumme>& tasten, double d) {
        iteration = it;
        summen = tasten;
        dauer = d;
    });
    REQUIRE(iteration.has_value());
    CHECK(summen.size() == 1);

    // die Datei der Iteration und die Wiederaufnahme daraus: x voll genau, λ, Nummer, Ende
    {
        const std::filesystem::path ordner = std::filesystem::temp_directory_path() / "lumiviz_test_suche_7510";
        std::filesystem::remove_all(ordner);
        std::filesystem::create_directories(ordner);
        REQUIRE(modelltest::schreibeIteration(ordner, 1, *iteration, a, summen, dauer));
        const std::optional<modelltest::LetzteIteration> l = modelltest::liesLetzteIteration(ordner, 1);
        REQUIRE(l.has_value());
        CHECK(l->nummer == 1);
        CHECK(l->x == iteration->x);  // bitgleich
        CHECK(l->lambda == iteration->lambda);
        CHECK(l->ende == iteration->ende);
        CHECK_FALSE(modelltest::liesLetzteIteration(ordner, 2).has_value());
        std::ifstream ein(modelltest::iterationsDatei(ordner, 1, 1));
        const std::string text((std::istreambuf_iterator<char>(ein)), std::istreambuf_iterator<char>());
        CHECK(text.find("s.abklingS = kurveAusPunkten({{60, ") != std::string::npos);
        CHECK(text.find("## Diagonale") != std::string::npos);
        ein.close();
        std::filesystem::remove_all(ordner);
    }
    CHECK((iteration->rest < iteration->restVorher || iteration->lambda > iteration->lambdaVorher));
    CHECK(iteration->diagonale.size() == 8);
    MESSAGE("Iteration an 60: Rest " << iteration->restVorher << " -> " << iteration->rest << ", lambda " << iteration->lambdaVorher << " -> "
                                     << iteration->lambda << ", " << iteration->versuche << " Versuche, " << s.auswertungen << " Auswertungen in "
                                     << dauer << " s (" << s.ergebnis.ende << ")");
    std::ostringstream d;
    d << modelltest::diagonaleText(a.raum, iteration->diagonale);
    MESSAGE("Diagonale von J^T J an 60:\n" << d.str());
}

TEST_CASE("Loop Salamander 7.5.10 Suche (LUMIVIZ_75_SUCHE)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_SUCHE");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_SUCHE=<Ordner> sucht an allen 30 Tasten von Startpunkt 1 und 2 aus (LUMIVIZ_75_SUCHE_START=1|2 nur einen); "
                "laeuft Stunden");
        return;
    }
    const std::filesystem::path ordner = variable.toStdU16String();
    std::filesystem::create_directories(ordner);
    const modelltest::SuchAufgabe a = aufgabe(modelltest::salamanderMidi());
    std::vector<int> starts{1, 2};
    if (const QString nur = qEnvironmentVariable("LUMIVIZ_75_SUCHE_START"); !nur.isEmpty()) starts = {nur.toInt()};
    const auto fortschritt = [&ordner](const std::string& zeile) {
        std::ofstream aus(ordner / "fortschritt.txt", std::ios::binary | std::ios::app);
        aus << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << '\t' << zeile << '\n';
    };

    // die Dauer ansagen: eine Auswertung gemessen, je Iteration rund zehn, hoechstens 20 Iterationen je Startpunkt
    {
        double sekunden = 0.0;
        (void)modelltest::bewerte(a, {modelltest::startpunkt(a.raum, 1)}, &sekunden);
        const double schaetzung = sekunden * 10.0 * 20.0 * static_cast<double>(starts.size());
        fortschritt("Start, " + std::string(compilerName()) + ", " + std::to_string(modelltest::poolThreads()) + " Threads, " +
                    std::to_string(a.raum.zahl()) + " Parameter; eine Auswertung " + zahl(sekunden, 1) + " s, geschaetzt hoechstens " +
                    zahl(schaetzung / 3600.0, 2) + " h");
        MESSAGE("Eine Auswertung " << sekunden << " s; die Suche dauert hoechstens rund " << schaetzung / 3600.0 << " h");
    }

    LmEinstellungen einst;
    for (const int start : starts)
    {
        const auto anfang = std::chrono::steady_clock::now();
        LmStart s{modelltest::startpunkt(a.raum, start), einst.lambdaStart};
        if (const std::optional<modelltest::LetzteIteration> l = modelltest::liesLetzteIteration(ordner, start); l.has_value())
        {
            if (!l->ende.empty() && std::filesystem::exists(ordner / ("ergebnis_" + std::to_string(start) + ".txt")))
            {
                fortschritt("Startpunkt " + std::to_string(start) + " schon fertig (" + l->ende + ")");
                continue;
            }
            s = LmStart{l->x, l->lambda, l->nummer};
            fortschritt("Startpunkt " + std::to_string(start) + " nimmt Iteration " + std::to_string(l->nummer) + " auf");
        }
        const TonSuche t = sucheAmTon(a, s, einst, [&](const LmIteration& it, const std::vector<TastenSumme>& tasten, double dauer) {
            REQUIRE(modelltest::schreibeIteration(ordner, start, it, a, tasten, dauer));
            const Gesamt g = gesamt(tasten);
            fortschritt("Startpunkt " + std::to_string(start) + " Iteration " + std::to_string(it.nummer) + ": Rest " + zahl(it.restVorher, 1) +
                        " -> " + zahl(it.rest, 1) + ", getroffen " + std::to_string(g.getroffen) + " / " + std::to_string(g.groessen) + ", lambda " +
                        zahl(it.lambda, 6) + ", " + std::to_string(it.verworfen) + " verworfen, " + zahl(dauer, 0) + " s" +
                        (it.ende.empty() ? "" : ", Ende: " + it.ende));
        });
        std::ofstream aus(ordner / ("ergebnis_" + std::to_string(start) + ".txt"), std::ios::binary);
        aus << "# Ergebnis der Suche 7.5.10, Startpunkt " << start << ", " << compilerName() << ", " << t.ergebnis.iterationen << " Iterationen, "
            << t.auswertungen << " Auswertungen in diesem Lauf, " << zahl(sekundenSeit(anfang), 0) << " s; Ende: " << t.ergebnis.ende << "\nrest\t"
            << t.ergebnis.rest << "\nan der Grenze\t" << modelltest::grenzenText(a.raum, modelltest::lmRaum(a.raum), t.ergebnis.x) << "\nx"
            << std::setprecision(17);
        for (const double v : t.ergebnis.x) aus << '\t' << v;
        aus << std::setprecision(6) << "\n\n## Kurven (wie baueVorgabe, direkt einsetzbar)\n" << modelltest::kurvenText(a.raum, t.ergebnis.x)
            << "\n## Je Taste und Groesse\n" << berichte(t.tasten, true);
        REQUIRE(static_cast<bool>(aus));
        fortschritt("Startpunkt " + std::to_string(start) + " fertig: " + t.ergebnis.ende + ", Rest " + zahl(t.ergebnis.rest, 1));
    }

    // beide fertig: der Vergleich je Stuetzpunkt und je Messgroesse (SP3 a: eindeutig, wenn beide an jeder Messgroesse
    // innerhalb der Toleranz enden)
    const std::optional<modelltest::LetzteIteration> e1 = modelltest::liesLetzteIteration(ordner, 1);
    const std::optional<modelltest::LetzteIteration> e2 = modelltest::liesLetzteIteration(ordner, 2);
    if (!e1 || !e2 || e1->ende.empty() || e2->ende.empty())
    {
        MESSAGE("Vergleich erst, wenn beide Startpunkte fertig sind");
        return;
    }
    const std::vector<modelltest::Bewertet> beide = modelltest::bewerte(a, {e1->x, e2->x});
    std::ostringstream v;
    v << std::fixed << std::setprecision(2) << "# Vergleich der Startpunkte 1 und 2 (SP3 a)\n## Je Stuetzpunkt: Kurve, Taste, Wert 1, Wert 2, "
      << "Abstand (log: %, linear: % des Bereichs)\n";
    std::size_t j = 0;
    double groesst = 0.0;
    for (const modelltest::SuchKurve& k : a.raum.kurven)
        for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
        {
            const double abstand = k.log ? 100.0 * (std::exp(std::abs(e1->x[j] - e2->x[j])) - 1.0)
                                         : 100.0 * std::abs(e1->x[j] - e2->x[j]) / (k.oben[i] - k.unten[i]);
            groesst = std::max(groesst, abstand);
            v << k.name << '\t' << k.tasten[i] << '\t' << std::setprecision(4) << k.wert(e1->x[j]) << '\t' << k.wert(e2->x[j]) << '\t'
              << std::setprecision(2) << abstand << '\n';
        }
    v << "\n## Je Messgroesse: Taste, Groesse, r 1, r 2, beide getroffen\n";
    int beideGetroffen = 0, alle = 0;
    for (std::size_t t = 0; t < beide[0].tasten.size(); ++t)
        for (std::size_t g = 0; g < beide[0].tasten[t].groessen.size(); ++g)
        {
            const Abweichung& a1 = beide[0].tasten[t].groessen[g];
            const Abweichung& a2 = beide[1].tasten[t].groessen[g];
            const bool ja = a1.getroffen && a2.getroffen;
            beideGetroffen += ja ? 1 : 0;
            ++alle;
            v << beide[0].tasten[t].midi << '\t' << a1.groesse << '\t' << a1.verhaeltnis << '\t' << a2.verhaeltnis << (ja ? "\tja" : "") << '\n';
        }
    std::ofstream(ordner / "vergleich.txt", std::ios::binary) << v.str();
    std::ofstream kurz(ordner / "kurzfassung.txt", std::ios::binary);
    kurz << std::fixed << std::setprecision(1) << "# Suche 7.5.10 (S162) — Kurzfassung\nStartpunkt 1: Rest " << quadratsumme(beide[0].r) << ", Ende "
         << e1->ende << ", " << e1->nummer << " Iterationen\nStartpunkt 2: Rest " << quadratsumme(beide[1].r) << ", Ende " << e2->ende << ", "
         << e2->nummer << " Iterationen\nbeide getroffen an " << beideGetroffen << " von " << alle << " Messgroessen (eindeutig nach SP3 a: "
         << (beideGetroffen == alle ? "ja" : "nein") << ")\ngroesster Abstand der Stuetzpunkte " << groesst << " %\n";
    MESSAGE("Vergleich: beide getroffen an " << beideGetroffen << " von " << alle << ", groesster Abstand " << groesst << " %");
}

TEST_CASE("Loop Salamander 7.5.10 Nelder-Mead gegen Levenberg-Marquardt (LUMIVIZ_75_NM)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_NM");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_NM=<Ordner> vergleicht Nelder-Mead und Levenberg-Marquardt an 33, 60, 84 je allein");
        return;
    }
    const std::filesystem::path ordner = variable.toStdU16String();
    std::filesystem::create_directories(ordner);
    std::ostringstream b;
    b << std::fixed << std::setprecision(3) << "# Nelder-Mead gegen Levenberg-Marquardt (ZF6 a, ZF9 a), " << compilerName() << ", "
      << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\n# je Taste allein, nur ihre Stuetzpunkte frei, ab Startpunkt 1; "
      << "Nelder-Mead hoechstens 150 Auswertungen, Anfangsschritt log ln 1,6, linear 0,25 des Bereichs\n"
      << "Taste | Parameter | Rest Start | NM Rest | NM Auswertungen | NM s | LM Rest | LM Auswertungen | LM Iterationen | LM s | LM Ende\n";
    for (const int taste : {33, 60, 84})
    {
        const modelltest::SuchAufgabe a = aufgabe({taste});
        const std::vector<double> x0 = modelltest::startpunkt(a.raum, 1);
        const auto f = [&a](const std::vector<double>& x) { return quadratsumme(modelltest::bewerte(a, {x}).front().r); };
        const double restStart = f(x0);
        std::vector<double> schritte;
        for (const modelltest::SuchKurve& k : a.raum.kurven)
            for (std::size_t i = 0; i < k.tasten.size(); ++i) schritte.push_back(k.log ? std::log(1.6) : 0.25 * (k.oben[i] - k.unten[i]));
        auto t0 = std::chrono::steady_clock::now();
        const NelderMead nm = nelderMead(f, x0, schritte, 150);
        const double nmS = sekundenSeit(t0);
        t0 = std::chrono::steady_clock::now();
        const TonSuche lm = sucheAmTon(a, LmStart{x0, LmEinstellungen{}.lambdaStart}, LmEinstellungen{});
        const double lmS = sekundenSeit(t0);
        b << taste << " | " << x0.size() << " | " << restStart << " | " << nm.rest << " | " << nm.auswertungen << " | " << nmS << " | " << lm.ergebnis.rest
          << " | " << lm.auswertungen << " | " << lm.ergebnis.iterationen << " | " << lmS << " | " << lm.ergebnis.ende << '\n';
        MESSAGE("Taste " << taste << ": Start " << restStart << ", NM " << nm.rest << " (" << nm.auswertungen << ", " << nmS << " s), LM " << lm.ergebnis.rest
                         << " (" << lm.auswertungen << ", " << lmS << " s)");
    }
    std::ofstream aus(ordner / "nm.txt", std::ios::binary);
    aus << b.str();
    REQUIRE(static_cast<bool>(aus));
}
