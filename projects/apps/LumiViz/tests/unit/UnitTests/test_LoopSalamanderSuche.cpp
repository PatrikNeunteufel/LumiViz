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
 * Probe 7.5.11 (S163):
 * - `LUMIVIZ_75_SCHNITT=<Ordner>` legt je Parameter einen Schnitt mit 15 Punkten ueber seinen Bereich (ZF8 a (2)) an den
 *   Tasten 33, 48, 60, 66, 81, 84 (`LUMIVIZ_75_SCHNITT_TASTEN=33,60`) — am Startpunkt 1 oder, mit
 *   `LUMIVIZ_75_SCHNITT_VON=<Suchordner>`, an der letzten Iteration von Startpunkt 1 dort; schreibt `schnitt.txt`.
 * - `LUMIVIZ_75_TASTE=<Ordner>` sucht mit Levenberg–Marquardt je Taste allein (nur ihre Stuetzpunkte frei) an denselben
 *   Tasten (`LUMIVIZ_75_TASTE_TASTEN`), ab Startpunkt 1 (`LUMIVIZ_75_TASTE_START=2`); schreibt `taste.txt`.
 * - `LUMIVIZ_75_DELTA_CHOR=1` gibt in diesen beiden Werkzeugen δ auch im Chor frei (`SuchOptionen::deltaImChor`).
 * 7.5.12 (V2 a, Bauauftrag `.claude/handover/auftrag_7.5.12.md`):
 * - `LUMIVIZ_75_SUCHE2=<Ordner>` sucht je Taste (A1–A5): je Startpunkt 1, 2, 3 (`LUMIVIZ_75_SUCHE2_START=1|2|3` nur einen)
 *   zuerst A4 (`a4_<start>.txt`), dann Levenberg–Marquardt je Taste (`iteration_<start>_<n>.txt`, `fortschritt.txt`), am
 *   Ende `a4_ende_<start>.txt` und `ergebnis_<start>.txt`; nach allen verlangten `ergebnis_bester.txt`, `vergleich.txt`
 *   (ab zwei Startpunkten) und `kurzfassung.txt`. `LUMIVIZ_75_SUCHE2_TASTEN=33,60` fuer Teilmengen. Nimmt auf wie die
 *   Suche 7.5.10. Laeuft Stunden — nur nach Ruecksprache mit Patrik. `LUMIVIZ_75_SUCHE2_OHNE_A4=1` (Gegenprobe S165):
 *   detune frei in der Suche statt fest nach A4.
 * - `LUMIVIZ_75_A4=1` setzt im Fall „detune an Taste 60“ detune am Ton (rund 30 s).
 * - `LUMIVIZ_75_SCHNITT_X=<Datei>` im Schnitt-Werkzeug: der Punkt aus der Zeile `x` einer Ergebnisdatei 7.5.12 mit ihrem
 *   `detune_fest`, Suchraum 7.5.12 (detune wird nicht geschnitten).
 * - `LUMIVIZ_75_PROBE12=<Suchordner>` urteilt nach HW3 a (mit `schnitt.txt` im selben Ordner) und schreibt `probe.txt`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "Hoerschwellen.hpp"
#include "LevenbergMarquardt.hpp"
#include "Modellton.hpp"
#include "NelderMead.hpp"
#include "Probe7512.hpp"
#include "Schwebungsstufe.hpp"
#include "Strukturmessung.hpp"
#include "Suche.hpp"
#include "Suchkurven.hpp"
#include "Zielsumme.hpp"

#include "InstrumentBody/Statistik.hpp"
#include "loop/HammerSaite.hpp"
#include "loop/HammerTon.hpp"

#include <QDateTime>
#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <numbers>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::messung;

namespace
{

const std::filesystem::path kStrukturDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_struktur.txt";
const std::filesystem::path kZielDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_ziel.txt";
const std::filesystem::path kHoerschwellenDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/hoerschwellen.txt";
const std::filesystem::path kProbenOrdner = "projects/apps/LumiViz/docs/modellierung/hammer-saite/proben";

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

/// Die Aufgabe an den `tasten` um die Vorgabe, mit Struktur und Ziel aus den mitgelieferten Dateien; die Optionen des
/// Suchraums nach 7.5.11 (`deltaImChor` in den Werkzeugen der Probe) und 7.5.12.
modelltest::SuchAufgabe aufgabe(const std::vector<int>& tasten, const modelltest::SuchOptionen& optionen = {})
{
    const std::optional<Strukturen> strukturen = liesStruktur(kStrukturDatei);
    const std::optional<Ziele> ziele = liesZiele(kZielDatei);
    REQUIRE(strukturen.has_value());
    REQUIRE(ziele.has_value());
    modelltest::SuchAufgabe a;
    a.tasten = tasten;
    a.strukturen = *strukturen;
    a.ziele = *ziele;
    a.raum = modelltest::suchraum(lumi::loop::hammerSaiteVorgabe(), tasten, a.strukturen, optionen);
    return a;
}

/// `LUMIVIZ_75_DELTA_CHOR=1` (Probe 7.5.11): δ auch im Chor frei.
bool deltaImChorVerlangt()
{
    return qEnvironmentVariable("LUMIVIZ_75_DELTA_CHOR") == QStringLiteral("1");
}

/// Die Tasten der Probe nach ZF8 a, oder die aus `variable` („33,60,84“).
std::vector<int> probeTasten(const char* variable)
{
    std::vector<int> tasten{33, 48, 60, 66, 81, 84};
    if (const QString t = qEnvironmentVariable(variable); !t.isEmpty())
    {
        tasten.clear();
        std::istringstream s(t.toStdString());
        std::string teil;
        while (std::getline(s, teil, ',')) tasten.push_back(std::stoi(teil));
    }
    return tasten;
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

TEST_CASE("Loop Salamander 7.5.11 Probe: Schnitt je Parameter (LUMIVIZ_75_SCHNITT)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_SCHNITT");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_SCHNITT=<Ordner> legt je Parameter einen Schnitt mit 15 Punkten an 33, 48, 60, 66, 81, 84 (ZF8 a (2)); "
                "LUMIVIZ_75_SCHNITT_VON=<Suchordner> nimmt die letzte Iteration von Startpunkt 1 als Punkt, LUMIVIZ_75_SCHNITT_X=<Datei> die Zeile x "
                "einer Ergebnisdatei der Suche 7.5.12 (rund 5 min)");
        return;
    }
    const std::filesystem::path ordner = variable.toStdU16String();
    std::filesystem::create_directories(ordner);
    const bool deltaChor = deltaImChorVerlangt();
    const QString vonX = qEnvironmentVariable("LUMIVIZ_75_SCHNITT_X");
    modelltest::SuchAufgabe alle =
        aufgabe(modelltest::salamanderMidi(), vonX.isEmpty() ? modelltest::SuchOptionen{deltaChor} : modelltest::suchOptionen7512());

    // der Punkt: Startpunkt 1, die letzte Iteration von Startpunkt 1 einer Suche oder (7.5.12) die Zeile x einer
    // Ergebnisdatei mit dem Suchraum 7.5.12 — detune ist dort fest und wird nicht geschnitten
    std::vector<double> x = modelltest::startpunkt(alle.raum, 1);
    std::string punkt = "Startpunkt 1";
    if (!vonX.isEmpty())
    {
        const std::filesystem::path pfad = vonX.toStdU16String();
        REQUIRE(modelltest::setzeDetuneAus(pfad, alle.raum));
        x = modelltest::liesX(pfad);
        REQUIRE(x.size() == alle.raum.zahl());
        std::ostringstream d;
        d << "Zeile x aus " << vonX.toStdString() << "; Suchraum 7.5.12 (delta ueberall frei ab 0,01 Cent, detune fest:";
        for (const int t : probeTasten("LUMIVIZ_75_SCHNITT_TASTEN")) d << ' ' << t << '=' << modelltest::festWert(alle.raum, "detune", t).value_or(0.0);
        d << " Cent)";
        punkt = d.str();
    }
    else if (const QString von = qEnvironmentVariable("LUMIVIZ_75_SCHNITT_VON"); !von.isEmpty())
    {
        const std::optional<modelltest::LetzteIteration> l = modelltest::liesLetzteIteration(std::filesystem::path(von.toStdU16String()), 1);
        REQUIRE(l.has_value());
        REQUIRE(l->x.size() == alle.raum.zahl());
        x = l->x;
        punkt = "Iteration " + std::to_string(l->nummer) + " von Startpunkt 1 aus " + von.toStdString();
    }

    constexpr int kPunkte = 15;
    const std::vector<int> tasten = probeTasten("LUMIVIZ_75_SCHNITT_TASTEN");
    std::ostringstream b;
    b << std::fixed << std::setprecision(3) << "# Probe 7.5.11, ZF8 a (2): Schnitt je Parameter mit " << kPunkte
      << " Punkten ueber seinen Bereich (die uebrigen Parameter am Punkt), " << compilerName() << ", "
      << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\n# Punkt: " << punkt << (deltaChor ? "; delta auch im Chor frei" : "")
      << "\n# Je Taste und Parameter: Wert am Punkt | die " << kPunkte << " Summen r^2 der Taste von unten nach oben | Spanne (groesste - kleinste) | "
      << "Minima (Rand zaehlt) | groesster Sprung zwischen Nachbarn gegen das Dreifache des mittleren | Residuen mit Kante (groesster Nachbarsprung "
      << "ueber 1 Toleranz und ueber dem Dreifachen ihres mittleren) | Wechsel fehlt/da\n# Glatt (ZF8 a (2)): hoechstens ein Minimum, Sprung <= 1\n";
    int glatt = 0;
    int parameter = 0;
    for (const int taste : tasten)
    {
        modelltest::SuchAufgabe a = alle;
        a.tasten = {taste};
        std::vector<std::vector<double>> punkte;
        std::vector<std::pair<std::size_t, std::size_t>> wer;  // Kurve, Parameter
        std::size_t j = 0;
        for (std::size_t k = 0; k < a.raum.kurven.size(); ++k)
        {
            const modelltest::SuchKurve& kurve = a.raum.kurven[k];
            for (std::size_t i = 0; i < kurve.tasten.size(); ++i, ++j)
            {
                if (kurve.tasten[i] != taste) continue;
                wer.emplace_back(k, j);
                const double u = kurve.koordinate(kurve.unten[i]);
                const double o = kurve.koordinate(kurve.oben[i]);
                for (int m = 0; m < kPunkte; ++m)
                {
                    std::vector<double> p = x;
                    p[j] = u + (o - u) * static_cast<double>(m) / (kPunkte - 1);
                    punkte.push_back(std::move(p));
                }
            }
        }
        punkte.push_back(x);
        const auto t0 = std::chrono::steady_clock::now();
        const std::vector<modelltest::Bewertet> bew = modelltest::bewerte(a, punkte);
        const double sekunden = sekundenSeit(t0);
        const modelltest::Bewertet& amPunkt = bew.back();
        const double summeAmPunkt = quadratsumme(amPunkt.r);
        b << "\n## Taste " << taste << ": Summe r^2 am Punkt " << summeAmPunkt << ", getroffen " << amPunkt.tasten.front().getroffen << " / " << amPunkt.r.size()
          << ", " << wer.size() << " Parameter, " << std::setprecision(0) << sekunden << " s\n" << std::setprecision(3);
        for (std::size_t w = 0; w < wer.size(); ++w)
        {
            const modelltest::SuchKurve& kurve = a.raum.kurven[wer[w].first];
            const auto bei = [&](int m) -> const modelltest::Bewertet& { return bew[w * kPunkte + static_cast<std::size_t>(m)]; };
            std::vector<double> s;
            for (int m = 0; m < kPunkte; ++m) s.push_back(quadratsumme(bei(m).r));
            int minima = 0;
            for (int m = 0; m < kPunkte; ++m)
            {
                const bool links = m == 0 || s[static_cast<std::size_t>(m)] < s[static_cast<std::size_t>(m - 1)];
                const bool rechts = m == kPunkte - 1 || s[static_cast<std::size_t>(m)] < s[static_cast<std::size_t>(m + 1)];
                if (links && rechts) ++minima;
            }
            double mittel = 0.0;
            double groesst = 0.0;
            for (int m = 0; m + 1 < kPunkte; ++m)
            {
                const double d = std::abs(s[static_cast<std::size_t>(m + 1)] - s[static_cast<std::size_t>(m)]);
                mittel += d;
                groesst = std::max(groesst, d);
            }
            mittel /= kPunkte - 1;
            const double sprung = mittel > 0.0 ? groesst / (3.0 * mittel) : 0.0;
            int kanten = 0;
            int wechsel = 0;
            for (std::size_t i = 0; i < amPunkt.r.size(); ++i)
            {
                double gr = 0.0;
                double mi = 0.0;
                int n = 0;
                for (int m = 0; m + 1 < kPunkte; ++m)
                {
                    const Abweichung& a1 = bei(m).tasten.front().groessen[i];
                    const Abweichung& a2 = bei(m + 1).tasten.front().groessen[i];
                    if (a1.fehlt != a2.fehlt)
                    {
                        ++wechsel;
                        continue;
                    }
                    const double d = std::abs(a2.residuum - a1.residuum);
                    gr = std::max(gr, d);
                    mi += d;
                    ++n;
                }
                if (n > 0 && gr > 1.0 && gr > 3.0 * mi / n) ++kanten;
            }
            const bool istGlatt = minima <= 1 && sprung <= 1.0;
            glatt += istGlatt ? 1 : 0;
            ++parameter;
            b << kurve.name << '\t' << kurve.wert(x[wer[w].second]) << "\t|";
            for (const double v : s) b << ' ' << v;
            b << "\t| Spanne " << *std::max_element(s.begin(), s.end()) - *std::min_element(s.begin(), s.end()) << " | Minima " << minima << " | Sprung " << sprung
              << " | Kanten " << kanten << " | Wechsel " << wechsel << (istGlatt ? " | glatt" : " | NICHT glatt") << '\n';
        }
        MESSAGE("Taste " << taste << ": " << wer.size() << " Parameter in " << sekunden << " s");
    }
    b << "\nGlatt nach ZF8 a (2): " << glatt << " / " << parameter << " Parameter\n";
    std::ofstream aus(ordner / "schnitt.txt", std::ios::binary);
    aus << b.str();
    REQUIRE(static_cast<bool>(aus));
    MESSAGE("Glatt " << glatt << " / " << parameter << ", Bericht " << (ordner / "schnitt.txt").string());
}

TEST_CASE("Loop Salamander 7.5.11 Probe: Levenberg-Marquardt je Taste allein (LUMIVIZ_75_TASTE)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_TASTE");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_TASTE=<Ordner> sucht je Taste allein an 33, 48, 60, 66, 81, 84 (LUMIVIZ_75_TASTE_TASTEN), ab Startpunkt 1 "
                "(LUMIVIZ_75_TASTE_START=2); mit LUMIVIZ_75_DELTA_CHOR=1 delta auch im Chor frei (rund 10 min)");
        return;
    }
    const std::filesystem::path ordner = variable.toStdU16String();
    std::filesystem::create_directories(ordner);
    const bool deltaChor = deltaImChorVerlangt();
    const int start = qEnvironmentVariable("LUMIVIZ_75_TASTE_START") == QStringLiteral("2") ? 2 : 1;
    std::ostringstream b;
    b << std::fixed << std::setprecision(3) << "# Probe 7.5.11: Levenberg-Marquardt je Taste allein (nur ihre Stuetzpunkte frei), " << compilerName() << ", "
      << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\n# Startpunkt " << start << (deltaChor ? "; delta auch im Chor frei" : "")
      << "\n# Taste | Parameter | Rest Start | Rest Ende | getroffen / alle | Iterationen | Auswertungen | s | Ende | an der Grenze\n";
    std::ostringstream einzeln;
    einzeln << std::setprecision(4);
    for (const int taste : probeTasten("LUMIVIZ_75_TASTE_TASTEN"))
    {
        const modelltest::SuchAufgabe a = aufgabe({taste}, modelltest::SuchOptionen{deltaChor});
        const std::vector<double> x0 = modelltest::startpunkt(a.raum, start);
        const double restStart = quadratsumme(modelltest::bewerte(a, {x0}).front().r);
        std::vector<double> diagonale;
        const TonSuche lm = sucheAmTon(a, LmStart{x0, LmEinstellungen{}.lambdaStart}, LmEinstellungen{},
                                       [&diagonale](const LmIteration& it, const std::vector<TastenSumme>&, double) { diagonale = it.diagonale; });
        const TastenSumme& t = lm.tasten.front();
        b << taste << " | " << x0.size() << " | " << restStart << " | " << lm.ergebnis.rest << " | " << t.getroffen << " / " << t.groessen.size() << " | "
          << lm.ergebnis.iterationen << " | " << lm.auswertungen << " | " << std::setprecision(0) << lm.sekunden << std::setprecision(3) << " | " << lm.ergebnis.ende
          << " | " << modelltest::grenzenText(a.raum, modelltest::lmRaum(a.raum), lm.ergebnis.x) << '\n';
        einzeln << "\n## Taste " << taste << " — Parameter am Ende (Kurve, Wert) und Wirkung (sqrt(diag) x Bereich, Toleranzen)\n";
        std::size_t j = 0;
        for (const modelltest::SuchKurve& k : a.raum.kurven)
            for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
                einzeln << k.name << '\t' << k.wert(lm.ergebnis.x[j]) << '\t' << (j < diagonale.size() ? std::sqrt(diagonale[j]) * k.bereich(i) : 0.0) << '\n';
        einzeln << berichte(lm.tasten, true);
        MESSAGE("Taste " << taste << ": Rest " << restStart << " -> " << lm.ergebnis.rest << ", getroffen " << t.getroffen << " / " << t.groessen.size() << " ("
                         << lm.ergebnis.iterationen << " Iterationen, " << lm.sekunden << " s, " << lm.ergebnis.ende << ")");
    }
    std::ofstream aus(ordner / "taste.txt", std::ios::binary);
    aus << b.str() << einzeln.str();
    REQUIRE(static_cast<bool>(aus));
}

namespace
{

/// Aufgaben von Hand fuer die Suche je Block: je Block eine Funktion seiner Parameter.
struct HandBloecke
{
    std::vector<LmBlock> bloecke;
    std::vector<std::function<std::vector<double>(const std::vector<double>&)>> f;
    std::vector<int> aufrufe;  ///< je Block, wie oft er ausgewertet wurde (Punkte)

    LmResiduenBloecke residuen()
    {
        return [this](const std::vector<std::vector<double>>& punkte, const std::vector<std::size_t>& welche) {
            std::vector<std::vector<double>> rs;
            for (const std::vector<double>& x : punkte)
                for (const std::size_t b : welche)
                {
                    std::vector<double> xb;
                    for (const std::size_t j : bloecke[b].parameter) xb.push_back(x[j]);
                    rs.push_back(f[b](xb));
                    ++aufrufe[b];
                }
            return rs;
        };
    }
    void dazu(std::vector<std::size_t> parameter, std::function<std::vector<double>(const std::vector<double>&)> funktion)
    {
        bloecke.push_back({std::move(parameter)});
        f.push_back(std::move(funktion));
        aufrufe.push_back(0);
    }
};

/// Der Teil von `v` an den Stellen `j`.
std::vector<double> teil(const std::vector<double>& v, const std::vector<std::size_t>& j)
{
    std::vector<double> t;
    for (const std::size_t i : j) t.push_back(v[i]);
    return t;
}

LmRaum teilRaum(const LmRaum& raum, const std::vector<std::size_t>& j)
{
    return {teil(raum.unten, j), teil(raum.oben, j), teil(raum.ende, j)};
}

}  // namespace

TEST_CASE("Loop Salamander 7.5.12 Suche je Block: gleich der Suche je Block allein")
{
    // drei unabhaengige Bloecke: das Abklingen mit Rauschen, atan ab 3 (Marquardt-Daempfung), Rosenbrock mit Grenze
    lumi::koerper::Zufall zufall(7);
    std::vector<double> t, y;
    for (int i = 0; i <= 50; ++i)
    {
        t.push_back(0.1 * i);
        y.push_back(2.0 * std::exp(-t.back() / 1.5) + 0.01 * zufall.normal());
    }
    HandBloecke h;
    h.dazu({0, 1}, [&](const std::vector<double>& x) {
        std::vector<double> r;
        for (std::size_t i = 0; i < t.size(); ++i) r.push_back((x[0] * std::exp(-t[i] / x[1]) - y[i]) / 0.01);
        return r;
    });
    h.dazu({2}, [](const std::vector<double>& x) { return std::vector<double>{std::atan(x[0])}; });
    h.dazu({3, 4}, [](const std::vector<double>& x) { return std::vector<double>{10.0 * (x[1] - x[0] * x[0]), 1.0 - x[0]}; });
    const LmRaum raum{{0.1, 0.1, -100.0, -2.0, -2.0}, {10.0, 10.0, 100.0, 0.5, 2.0}, {1e-8, 1e-8, 1e-10, 1e-9, 1e-9}};
    const std::vector<double> differenz(5, 1e-7);
    const std::vector<double> x0{1.0, 1.0, 3.0, -1.2, 1.0};
    LmEinstellungen einst;
    einst.iterationenHoechstens = 200;
    const LmErgebnisBloecke e = levenbergMarquardtJeBlock(h.residuen(), h.bloecke, {{0, 2, 3}, {1, 4}}, differenz, LmStartBloecke{x0}, raum, einst);

    for (std::size_t b = 0; b < h.bloecke.size(); ++b)
    {
        CAPTURE(b);
        const std::vector<std::size_t>& p = h.bloecke[b].parameter;
        const LmRaum rb = teilRaum(raum, p);
        const LmResiduenViele viele = [&](const std::vector<std::vector<double>>& punkte) {
            std::vector<std::vector<double>> rs;
            for (const auto& x : punkte) rs.push_back(h.f[b](x));
            return rs;
        };
        const LmJacobi jacobi = [&](const std::vector<double>& x, const std::vector<double>& r) {
            return jacobiEinzeln(viele, x, r, differenzSchritte(rb, x, teil(differenz, p)));
        };
        const LmErgebnis allein = levenbergMarquardt(h.f[b], jacobi, LmStart{teil(x0, p)}, rb, einst);
        const LmBlockStand& st = e.bloecke[b];
        CHECK(teil(e.x, p) == allein.x);  // bitgleich
        CHECK(st.rest == allein.rest);
        CHECK(st.lambda == allein.lambda);
        CHECK(st.iterationen == allein.iterationen);
        CHECK(st.ende == allein.ende);
        CHECK(st.auswertungen == allein.auswertungen);
        MESSAGE("Block " << b << ": " << st.iterationen << " Iterationen, Rest " << st.rest << " (" << st.ende << ")");
    }
    CHECK(e.iterationen == std::max({e.bloecke[0].iterationen, e.bloecke[1].iterationen, e.bloecke[2].iterationen}));
    REQUIRE(e.anGrenze.size() == 1);
    CHECK(e.anGrenze.front() == 3);  // Rosenbrock x0 an der Grenze 0,5
}

TEST_CASE("Loop Salamander 7.5.12 Suche je Block: eigenes lambda und Ende je Block")
{
    // atan ab 3 verwirft im ersten Schritt viermal (λ steigt), die Gerade r = x − 0,5 nimmt gleich an (λ faellt) und
    // endet in der zweiten Iteration mit „Schritte klein“ (Ende 0,01)
    HandBloecke h;
    h.dazu({0}, [](const std::vector<double>& x) { return std::vector<double>{std::atan(x[0])}; });
    h.dazu({1}, [](const std::vector<double>& x) { return std::vector<double>{x[0] - 0.5}; });
    const LmRaum raum{{-100.0, -10.0}, {100.0, 10.0}, {1e-10, 0.01}};
    std::vector<LmIterationBloecke> iterationen;
    std::vector<std::vector<int>> aufrufeNach;
    const LmErgebnisBloecke e = levenbergMarquardtJeBlock(h.residuen(), h.bloecke, {{0, 1}}, {1e-7, 1e-7}, LmStartBloecke{{3.0, 0.0}}, raum, {},
                                                          [&](const LmIterationBloecke& it) {
                                                              iterationen.push_back(it);
                                                              aufrufeNach.push_back(h.aufrufe);
                                                          });
    REQUIRE(iterationen.size() >= 3);
    const LmIterationBloecke& erste = iterationen.front();
    CHECK(erste.bloecke[0].verworfen >= 2);
    CHECK(erste.bloecke[0].lambda > erste.bloecke[0].lambdaVorher);
    CHECK(erste.bloecke[1].verworfen == 0);
    CHECK(erste.bloecke[1].lambda < erste.bloecke[1].lambdaVorher);
    for (const LmIterationBloecke& it : iterationen)
        for (std::size_t b = 0; b < 2; ++b)
        {
            CAPTURE(it.nummer);
            CAPTURE(b);
            const LmBlockSchritt& s = it.bloecke[b];
            if (!s.aktiv) continue;
            CHECK(s.lambda == doctest::Approx(s.lambdaVorher * std::pow(10.0, s.verworfen - (s.angenommen ? 1 : 0))));
            CHECK(s.versuche == s.verworfen + (s.angenommen ? 1 : 0));
            if (s.angenommen) CHECK(s.rest < s.restVorher);
        }

    // die Gerade endet in Iteration 2 und wird danach nicht mehr ausgewertet; atan laeuft weiter
    CHECK(e.bloecke[1].ende == "Schritte klein");
    CHECK(e.bloecke[1].iterationen == 2);
    CHECK(iterationen[1].bloecke[1].ende == "Schritte klein");
    for (std::size_t i = 2; i < iterationen.size(); ++i)
    {
        CHECK_FALSE(iterationen[i].bloecke[1].aktiv);
        CHECK(aufrufeNach[i][1] == aufrufeNach[1][1]);
        CHECK(aufrufeNach[i][0] > aufrufeNach[i - 1][0]);
    }
    CHECK(e.bloecke[0].iterationen > 2);
    CHECK(e.bloecke[0].ende == "Schritte klein");
    CHECK(std::abs(e.x[0]) < 1e-6);
    MESSAGE("atan " << e.bloecke[0].iterationen << " Iterationen, Gerade " << e.bloecke[1].iterationen << "; Aufrufe " << h.aufrufe[0] << " / " << h.aufrufe[1]);
}

TEST_CASE("Loop Salamander 7.5.12 Suche je Block: kein Schritt ohne Wirkung (A3)")
{
    // drei Parameter im Bereich 0…1: p0 wirkt nicht, p1 wirkt 0,05 Toleranzen, p2 0,5 — nur p2 bewegt sich
    HandBloecke h;
    h.dazu({0, 1, 2}, [](const std::vector<double>& x) { return std::vector<double>{0.05 * (x[1] - 0.9), 0.5 * (x[2] - 0.9)}; });
    const LmRaum raum{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, {1e-6, 1e-6, 1e-6}};
    std::vector<LmIterationBloecke> iterationen;
    const LmErgebnisBloecke e = levenbergMarquardtJeBlock(h.residuen(), h.bloecke, {{0}, {1}, {2}}, {1e-6, 1e-6, 1e-6}, LmStartBloecke{{0.2, 0.2, 0.2}},
                                                          raum, {}, [&](const LmIterationBloecke& it) { iterationen.push_back(it); });
    CHECK(e.x[0] == 0.2);  // bitgleich am Start
    CHECK(e.x[1] == 0.2);
    CHECK(e.x[2] == doctest::Approx(0.9).epsilon(1e-6));
    REQUIRE_FALSE(iterationen.empty());
    for (const LmIterationBloecke& it : iterationen)
    {
        CAPTURE(it.nummer);
        CHECK(it.bloecke[0].ohneWirkung == std::vector<std::size_t>{0, 1});
        CHECK(std::sqrt(it.bloecke[0].diagonale[1]) == doctest::Approx(0.05).epsilon(1e-6));
    }
    CHECK(e.bloecke[0].ohneWirkung == std::vector<std::size_t>{0, 1});

    // der Schritt mit Maske: alle frei gibt bitgleich den alten, eine gestrichene Spalte gibt dort 0 und sonst den Schritt
    // des Systems ohne sie
    Matrix j(3, 2);
    j(0, 0) = 1.5;
    j(0, 1) = -0.3;
    j(1, 0) = 0.2;
    j(1, 1) = 2.0;
    j(2, 0) = -0.7;
    j(2, 1) = 0.4;
    const std::vector<double> r{0.3, -1.1, 0.8};
    CHECK(lmSchritt(j, r, 0.01, {true, true}) == lmSchritt(j, r, 0.01));
    Matrix j1(3, 1);
    for (std::size_t i = 0; i < 3; ++i) j1(i, 0) = j(i, 1);
    const std::vector<double> nurZweite = lmSchritt(j, r, 0.01, {false, true});
    CHECK(nurZweite[0] == 0.0);
    CHECK(nurZweite[1] == lmSchritt(j1, r, 0.01)[0]);
    CHECK(lmSchritt(j, r, 0.01, {false, false}) == std::vector<double>{0.0, 0.0});
}

namespace
{

const modelltest::SuchKurve& kurveNamens(const modelltest::Suchraum& raum, const std::string& name)
{
    for (const modelltest::SuchKurve& k : raum.kurven)
        if (k.name == name) return k;
    FAIL("keine Kurve " << name);
    return raum.kurven.front();
}

/// Der Index des Stuetzpunkts der Kurve `name` an `taste` in x.
std::size_t parameterIndex(const modelltest::Suchraum& raum, const std::string& name, int taste)
{
    for (std::size_t k = 0; k < raum.kurven.size(); ++k)
        if (raum.kurven[k].name == name)
            for (std::size_t i = 0; i < raum.kurven[k].tasten.size(); ++i)
                if (raum.kurven[k].tasten[i] == taste) return raum.ab(k) + i;
    FAIL("kein Stuetzpunkt " << name << " an " << taste);
    return 0;
}

}  // namespace

TEST_CASE("Loop Salamander 7.5.12 Suchraum: delta an allen Tasten frei, detune fest, Startpunkt 3")
{
    const std::vector<int> tasten = modelltest::salamanderMidi();
    const modelltest::SuchAufgabe alt = aufgabe(tasten);
    const modelltest::SuchAufgabe neu = aufgabe(tasten, modelltest::suchOptionen7512());
    const lumi::loop::HammerSaite& vorgabe = lumi::loop::hammerSaiteVorgabe();

    // die Vorgabe {} ist der Raum von 7.5.9 (im Einzelnen prueft ihn „7.5.9 Ziel: Kurven der Suche“)
    CHECK(alt.raum.zahl() == 5 * 30 + 3 * 26 + 4);
    CHECK(kurveNamens(alt.raum, "delta").unten == std::vector<double>(4, 0.0));
    // A1: δ an allen 30 Tasten frei, unten 0,01 Cent, der Start in den Grenzen; A4: detune nicht frei
    const modelltest::SuchKurve& delta = kurveNamens(neu.raum, "delta");
    CHECK(delta.tasten.size() == 30);
    CHECK(delta.festTasten.empty());
    for (std::size_t i = 0; i < delta.tasten.size(); ++i)
    {
        CAPTURE(delta.tasten[i]);
        CHECK(delta.unten[i] == 0.01);
        CHECK(delta.oben[i] == 8.0);
        CHECK(delta.start[i] >= 0.01);
        CHECK(delta.start[i] <= 8.0);
    }
    const modelltest::SuchKurve& detune = kurveNamens(neu.raum, "detune");
    const modelltest::SuchKurve& detuneAlt = kurveNamens(alt.raum, "detune");
    CHECK(detune.tasten.empty());
    REQUIRE(detune.festTasten.size() == 30);
    CHECK(neu.raum.zahl() == 5 * 30 + 2 * 26 + 30);
    for (std::size_t i = 0; i < detune.festTasten.size(); ++i)
    {
        const int t = detune.festTasten[i];
        CAPTURE(t);
        const auto frei = std::find(detuneAlt.tasten.begin(), detuneAlt.tasten.end(), t);
        if (frei != detuneAlt.tasten.end())
            CHECK(detune.festWerte[i] == detuneAlt.start[static_cast<std::size_t>(frei - detuneAlt.tasten.begin())]);  // Start nach SG3 a
        else
            CHECK(detune.festWerte[i] == lumi::loop::wertAn(vorgabe.verstimmungCent, t, lumi::loop::KurvenSkala::Linear));
    }
    // die uebrigen Kurven gleich
    for (std::size_t k = 0; k < alt.raum.kurven.size(); ++k)
    {
        const modelltest::SuchKurve& a = alt.raum.kurven[k];
        const modelltest::SuchKurve& n = neu.raum.kurven[k];
        CAPTURE(a.name);
        CHECK(a.name == n.name);
        if (a.name == "delta" || a.name == "detune") continue;
        CHECK(a.tasten == n.tasten);
        CHECK(a.unten == n.unten);
        CHECK(a.oben == n.oben);
        CHECK(a.start == n.start);
        CHECK(a.festWerte == n.festWerte);
    }
    // die Saite aus Startpunkt 1 traegt detune am Start nach SG3 a
    CHECK(lumi::loop::wertAn(modelltest::saiteAus(neu.raum, modelltest::startpunkt(neu.raum, 1)).verstimmungCent, 60, lumi::loop::KurvenSkala::Linear) ==
          doctest::Approx(*modelltest::festWert(neu.raum, "detune", 60)));
    // setzeFest aendert den festen Wert, nur wo die Taste fest ist
    {
        modelltest::Suchraum r = neu.raum;
        CHECK(modelltest::setzeFest(r, "detune", 60, 1.25));
        CHECK(*modelltest::festWert(r, "detune", 60) == 1.25);
        CHECK_FALSE(modelltest::setzeFest(r, "decay", 60, 1.0));
    }

    // A5: Startpunkt 3 ist das Spiegelbild von 2 um 1 (vor dem Klemmen), in den Grenzen; 1 und 2 wie bisher
    for (const modelltest::SuchAufgabe* a : {&alt, &neu})
    {
        const std::vector<double> x1 = modelltest::startpunkt(a->raum, 1);
        const std::vector<double> x2 = modelltest::startpunkt(a->raum, 2);
        const std::vector<double> x3 = modelltest::startpunkt(a->raum, 3);
        REQUIRE(x3.size() == a->raum.zahl());
        CHECK(x3 == modelltest::klemme(a->raum, x3));
        std::size_t j = 0;
        int geklemmt = 0;
        for (const modelltest::SuchKurve& k : a->raum.kurven)
            for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
            {
                CAPTURE(k.name);
                CAPTURE(k.tasten[i]);
                const double c = k.koordinate(k.start[i]);
                const double d = k.log ? std::log(1.6) : 0.25 * (k.oben[i] - k.unten[i]);
                const double u = k.koordinate(k.unten[i]);
                const double o = k.koordinate(k.oben[i]);
                CHECK(x1[j] == std::clamp(c, u, o));
                CHECK(x2[j] == std::clamp(c + d, u, o));
                CHECK(x3[j] == std::clamp(c - d, u, o));
                geklemmt += x3[j] != c - d ? 1 : 0;
            }
        MESSAGE("Startpunkt 3: " << geklemmt << " von " << j << " Parametern an einer Grenze");
    }
}

TEST_CASE("Loop Salamander 7.5.12 Suchraum: keine Entartung an 84 mit delta 0,01 Cent")
{
    // A1: mit δ = 0,01 Cent haengt Σ r² stetig an h (S163: bei δ = 0 und h = 1 rauscht die entartete Eigenzerlegung)
    const modelltest::SuchAufgabe a = aufgabe({84}, modelltest::suchOptionen7512());
    modelltest::SuchOptionen ohne = modelltest::suchOptionen7512();
    ohne.deltaUntenCent = 0.0;
    const modelltest::SuchAufgabe b = aufgabe({84}, ohne);
    const std::size_t jd = parameterIndex(a.raum, "delta", 84);
    const std::size_t jh = parameterIndex(a.raum, "h", 84);
    const std::vector<double> x = modelltest::startpunkt(a.raum, 1);
    const auto punkt = [&](double delta, double h) {
        std::vector<double> p = x;
        p[jd] = delta;
        p[jh] = h;
        return p;
    };
    const auto anfang = std::chrono::steady_clock::now();
    const std::vector<modelltest::Bewertet> mit = modelltest::bewerte(a, {punkt(0.01, 1.0), punkt(0.01, 1.0 - 1e-6)});
    const std::vector<modelltest::Bewertet> null = modelltest::bewerte(b, {punkt(0.0, 1.0), punkt(0.0, 1.0 - 1e-6)});
    const auto relativ = [](const std::vector<modelltest::Bewertet>& v) {
        const double s1 = quadratsumme(v[0].r);
        const double s2 = quadratsumme(v[1].r);
        return std::abs(s1 - s2) / std::max({s1, s2, 1e-300});
    };
    MESSAGE("an 84, Summe r^2 bei h = 1 und 1 - 1e-6: delta 0,01 Cent " << quadratsumme(mit[0].r) << " / " << quadratsumme(mit[1].r) << " (relativ "
                                                                     << relativ(mit) << "); delta 0 " << quadratsumme(null[0].r) << " / "
                                                                     << quadratsumme(null[1].r) << " (relativ " << relativ(null) << "), "
                                                                     << sekundenSeit(anfang) << " s");
    CHECK(relativ(mit) < 0.01);

    // die Rechnung: der Abstand der Eigenwerte beider Ebenen gegen die Schwelle des Nachschaerfens in `zerlegeKlein`
    // (abstand > 1e-8 · ‖a‖_F, Eigenzerlegung.cpp). `a` ist in `ebenenModen` (HammerTon.cpp) die 2N × 2N-Matrix des
    // Chors um die mittlere Kreisfrequenz verschoben: Diagonale −σ_Saite + i(ω − Mitte), die zweite Ebene um δ hoeher,
    // dazu −σ_V·s·sᵀ − σ_H·w·wᵀ am Steg. ‖a‖_F ist hier aus den Eigenwerten geschaetzt (√Σ|λ − i·Mitte|², fuer eine
    // nahezu normale Matrix gleich, sonst eine untere Grenze — der Faktor also eher zu gross).
    const double relativDelta = std::exp2(0.01 / 1200.0) - 1.0;
    lumi::loop::HammerAnschlag anschlag;
    anschlag.taste = 84;
    anschlag.hz = tastenFrequenz(84);
    const lumi::loop::HammerSaite saite = modelltest::saiteAus(a.raum, punkt(0.01, 1.0));
    double kleinsterFaktor = std::numeric_limits<double>::infinity();
    for (const lumi::loop::TeiltonModen& t : lumi::loop::hammerTeiltonModen(saite, anschlag))
    {
        if (t.teilton > 3 || t.moden.size() < 2) continue;
        double mitte = 0.0;
        for (const lumi::loop::EbenenMode& m : t.moden) mitte += m.lambda.imag();
        mitte /= static_cast<double>(t.moden.size());
        double norm = 0.0;
        double abstand = std::numeric_limits<double>::infinity();
        for (std::size_t i = 0; i < t.moden.size(); ++i)
        {
            norm += std::norm(t.moden[i].lambda - std::complex<double>(0.0, mitte));
            for (std::size_t j = i + 1; j < t.moden.size(); ++j) abstand = std::min(abstand, std::abs(t.moden[i].lambda - t.moden[j].lambda));
        }
        norm = std::sqrt(norm);
        const double faktor = abstand / (1e-8 * norm);
        kleinsterFaktor = std::min(kleinsterFaktor, faktor);
        MESSAGE("Teilton " << t.teilton << " (" << t.hz << " Hz, " << t.moden.size() << " Moden): delta 0,01 Cent relativ " << relativDelta << ", also "
                           << 2.0 * std::numbers::pi * t.hz * relativDelta << " 1/s; kleinster Abstand der Eigenwerte " << abstand << " 1/s, ||a||_F ~ "
                           << norm << " 1/s, Schwelle " << 1e-8 * norm << " 1/s — Faktor " << faktor);
    }
    MESSAGE("kleinster Faktor zur Schwelle " << kleinsterFaktor << std::string(kleinsterFaktor < 10.0 ? " (unter 10: Frage fuers Handover)" : ""));
}

TEST_CASE("Loop Salamander 7.5.12 Suchraum: der beste Lauf je Taste")
{
    // zwei „Laeufe“ an 33 und 60: Startpunkt 1, und Startpunkt 2 mit anderem festem detune (wie nach A4); zusammengesetzt
    // gibt die Auswertung je Taste bitgleich die Summe des Laufs, aus dem die Taste kommt
    const std::vector<int> tasten{33, 60};
    modelltest::SuchAufgabe a1 = aufgabe(tasten, modelltest::suchOptionen7512());
    modelltest::SuchAufgabe a2 = a1;
    REQUIRE(modelltest::setzeFest(a2.raum, "detune", 33, 1.0));
    REQUIRE(modelltest::setzeFest(a2.raum, "detune", 60, 2.5));
    const std::vector<double> x1 = modelltest::startpunkt(a1.raum, 1);
    const std::vector<double> x2 = modelltest::startpunkt(a2.raum, 2);
    const modelltest::Bewertet b1 = modelltest::bewerte(a1, {x1}).front();
    const modelltest::Bewertet b2 = modelltest::bewerte(a2, {x2}).front();
    const auto summen = [](const modelltest::Bewertet& b) {
        std::vector<double> s;
        for (const TastenSumme& t : b.tasten) s.push_back(t.summe);
        return s;
    };
    const std::vector<modelltest::SuchLauf> laeufe{{a1.raum, x1, summen(b1)}, {a2.raum, x2, summen(b2)}};
    const auto pruefe = [&](const modelltest::BesterJeTaste& best, const std::vector<std::size_t>& herkunft) {
        CHECK(best.herkunft == herkunft);
        modelltest::SuchAufgabe ab = a1;
        ab.raum = best.raum;
        const modelltest::Bewertet bb = modelltest::bewerte(ab, {best.x}).front();
        for (std::size_t t = 0; t < tasten.size(); ++t)
        {
            CAPTURE(tasten[t]);
            const modelltest::Bewertet& quelle = herkunft[t] == 0 ? b1 : b2;
            CHECK(bb.tasten[t].summe == quelle.tasten[t].summe);  // bitgleich
            CHECK(*modelltest::festWert(best.raum, "detune", tasten[t]) == *modelltest::festWert(laeufe[herkunft[t]].raum, "detune", tasten[t]));
        }
    };
    // nach den echten Summen
    std::vector<std::size_t> erwartet;
    for (std::size_t t = 0; t < tasten.size(); ++t) erwartet.push_back(b2.tasten[t].summe < b1.tasten[t].summe ? 1 : 0);
    pruefe(modelltest::besterJeTaste(laeufe, tasten), erwartet);
    // gemischt erzwungen: 33 aus dem ersten, 60 aus dem zweiten (die Summen nur zur Wahl vertauscht)
    std::vector<modelltest::SuchLauf> gemischt = laeufe;
    gemischt[0].summeJeTaste = {0.0, 1.0};
    gemischt[1].summeJeTaste = {1.0, 0.0};
    pruefe(modelltest::besterJeTaste(gemischt, tasten), {0, 1});
    // Gleichstand: der fruehere
    gemischt[1].summeJeTaste = {0.0, 1.0};
    CHECK(modelltest::besterJeTaste(gemischt, tasten).herkunft == std::vector<std::size_t>{0, 0});
    MESSAGE("Summen Lauf 1: " << b1.tasten[0].summe << " / " << b1.tasten[1].summe << ", Lauf 2: " << b2.tasten[0].summe << " / " << b2.tasten[1].summe);
}

TEST_CASE("Loop Salamander 7.5.12 detune: Raster und Goldener Schnitt, Abstand der Schwebung, Datei")
{
    // drei Aufgaben zugleich: zwei Minima (das globale bei 4,37, das oertliche bei 1,3 mit 0,5), eines am Rand (0), eines
    // nahe dem oberen Rand eines laengeren Bereichs (7,2 von 7,5)
    const std::vector<double> unten{0.0, 0.0, 0.0};
    const std::vector<double> oben{6.0, 6.0, 7.5};
    const std::vector<std::function<double(double)>> f{
        [](double x) { return std::min((x - 1.3) * (x - 1.3) + 0.5, 2.0 * (x - 4.37) * (x - 4.37)); },
        [](double x) { return 0.3 * x + 0.2 * std::sin(3.0 * x) * std::sin(3.0 * x); },
        [](double x) { return std::abs(x - 7.2) + 0.1 * std::cos(5.0 * x); },
    };
    std::vector<std::size_t> spaltenJeAufruf;
    const modelltest::Ziel1DSpalten ziel = [&](const std::vector<std::vector<double>>& spalten) {
        spaltenJeAufruf.push_back(spalten.size());
        std::vector<std::vector<double>> g;
        for (const std::vector<double>& s : spalten)
        {
            g.emplace_back();
            for (std::size_t i = 0; i < s.size(); ++i) g.back().push_back(f[i](s[i]));
        }
        return g;
    };
    int aufrufe = 0;
    const std::vector<modelltest::Minimum1D> m = modelltest::minimiere1D(unten, oben, ziel, 15, 0.01, 8, &aufrufe);
    REQUIRE(m.size() == 3);
    CHECK(std::abs(m[0].x - 4.37) <= 0.01);
    CHECK(m[0].g == doctest::Approx(f[0](m[0].x)));
    CHECK(m[1].x == 0.0);
    // das dritte: |x − 7,2| + 0,1·cos 5x hat sein Minimum bei 7,2 ± wenige Hundertstel; das Gefundene ist nicht schlechter
    // als jeder Punkt eines feinen Rasters um 7,2 bis auf 0,01
    double feinBest = 1e9;
    for (int i = -100; i <= 100; ++i) feinBest = std::min(feinBest, f[2](7.2 + 0.001 * i));
    CHECK(m[2].g <= feinBest + 0.01);
    CHECK(aufrufe == static_cast<int>(spaltenJeAufruf.size()));
    CHECK(spaltenJeAufruf.front() == 15);
    CHECK(spaltenJeAufruf[1] == 2);
    CHECK(aufrufe <= 1 + 1 + 8 + 1);
    CHECK(m[0].auswertungen == static_cast<int>(std::accumulate(spaltenJeAufruf.begin(), spaltenJeAufruf.end(), std::size_t{0})));
    MESSAGE("1D: " << m[0].x << " (g " << m[0].g << "), " << m[1].x << ", " << m[2].x << "; " << aufrufe << " Aufrufe, " << m[0].auswertungen
                   << " Auswertungen je Aufgabe");

    // der Abstand der Schwebung: Teilton 1 im Ziel 2 Hz, im Modell 2,2; Teilton 2 schwebt im Ziel nicht (zaehlt nicht);
    // Teilton 3 im Ziel 3 Hz, im Modell keine — (ln 1,1)² + (ln 3)²
    TonStruktur zielS{};
    TonStruktur modellS{};
    for (std::size_t k = 0; k < 3; ++k) zielS[k].da = modellS[k].da = true;
    zielS[0].schwebt = true;
    zielS[0].schwebungHz = 2.0;
    zielS[2].schwebt = true;
    zielS[2].schwebungHz = 3.0;
    modellS[0].schwebt = true;
    modellS[0].schwebungHz = 2.2;
    modellS[1].schwebt = true;
    modellS[1].schwebungHz = 5.0;
    CHECK(modelltest::schwebungsAbstand(modellS, zielS) == doctest::Approx(std::log(1.1) * std::log(1.1) + std::log(3.0) * std::log(3.0)));
    CHECK(modelltest::schwebtImZiel(zielS));
    CHECK_FALSE(modelltest::schwebtImZiel(TonStruktur{}));

    // die Datei a4_<start>.txt hin und zurueck
    std::vector<modelltest::DetuneTaste> tasten(2);
    tasten[0].taste = 60;
    tasten[0].gesetzt = true;
    tasten[0].vorher = 1.234567890123;
    tasten[0].nachher = 0.5;
    tasten[0].gVorher = 0.25;
    tasten[0].gNachher = 0.01;
    tasten[0].zielHz = {1.5, 0.0, 2.25};
    tasten[0].modellNachherHz = {1.49, 0.0, 2.3};
    tasten[1].taste = 21;
    tasten[1].grund = "eine Saite";
    const std::filesystem::path pfad = std::filesystem::temp_directory_path() / "lumiviz_test_a4_7512.txt";
    std::ofstream(pfad, std::ios::binary) << modelltest::stufeText(tasten);
    const std::optional<std::vector<modelltest::DetuneTaste>> zurueck = modelltest::liesStufe(pfad);
    std::filesystem::remove(pfad);
    REQUIRE(zurueck.has_value());
    REQUIRE(zurueck->size() == 2);
    CHECK((*zurueck)[0].vorher == tasten[0].vorher);
    CHECK((*zurueck)[0].nachher == 0.5);
    CHECK((*zurueck)[0].zielHz == tasten[0].zielHz);
    CHECK((*zurueck)[0].modellNachherHz == tasten[0].modellNachherHz);
    CHECK((*zurueck)[0].gesetzt);
    CHECK_FALSE((*zurueck)[1].gesetzt);
    CHECK((*zurueck)[1].grund == "eine Saite");
    CHECK((*zurueck)[0].grund.empty());
}

TEST_CASE("Loop Salamander 7.5.12 detune: an Taste 60 am Ton (LUMIVIZ_75_A4)")
{
    if (qEnvironmentVariable("LUMIVIZ_75_A4") != QStringLiteral("1"))
    {
        MESSAGE("LUMIVIZ_75_A4=1 setzt detune an Taste 60 am Ton an die Schwebung von Salamander (rund 1-2 min)");
        return;
    }
    const modelltest::SuchAufgabe a = aufgabe({60}, modelltest::suchOptionen7512());
    const std::vector<double> x = modelltest::startpunkt(a.raum, 1);
    const modelltest::DetuneStufe st = modelltest::stufeDetune(a, x);
    REQUIRE(st.tasten.size() == 1);
    const modelltest::DetuneTaste& d = st.tasten.front();
    REQUIRE(d.gesetzt);
    CHECK(d.gNachher <= d.gVorher);
    CHECK(*modelltest::festWert(st.raum, "detune", 60) == d.nachher);
    std::ostringstream hz;
    for (std::size_t k = 0; k < 3; ++k)
        hz << " | k" << k + 1 << ": Ziel " << d.zielHz[k] << ", Modell vorher " << d.modellVorherHz[k] << ", nachher " << d.modellNachherHz[k];
    MESSAGE("Taste 60: detune " << d.vorher << " -> " << d.nachher << " Cent, g " << d.gVorher << " -> " << d.gNachher << hz.str() << "; " << st.aufrufe
                                << " Aufrufe, " << st.auswertungen << " Auswertungen, " << st.sekunden << " s");
}

TEST_CASE("Loop Salamander 7.5.12 Hoerschwellen: lesen und anwenden (HW1 a, HW2 a)")
{
    const HoerschwellenGelesen g = liesHoerschwellen(kHoerschwellenDatei);
    INFO(g.fehler);
    REQUIRE(g.werte.has_value());
    const Hoerschwellen& h = *g.werte;
    CHECK(h.zeilen.size() == 7);
    const auto muster = [&h](const std::string& groesse) { return h.fuer(groesse) != nullptr ? h.fuer(groesse)->muster : std::string("-"); };
    CHECK(muster("t60_spaet_2") == "t60_spaet");
    CHECK(muster("vorwaerts_t60_3") == "vorwaerts_t60");
    CHECK(muster("huelle_30_s") == "huelle_");
    CHECK(muster("knick_tiefe_db_2") == "knick_tiefe_db");
    CHECK(muster("tiefe_db_3") == "tiefe_db");
    CHECK(muster("schwebung_db_2") == "schwebung_db");
    CHECK(muster("schwebung_hz") == "schwebung_hz");
    CHECK(muster("unbekannt") == "-");
    CHECK(h.fuer("t60_spaet")->skala == HoerSkala::Faktor);
    CHECK(h.fuer("tiefe_db_2")->skala == HoerSkala::Ewert);
    CHECK(h.fuer("knick_tiefe_db")->unten == 0.5);
    // eine fehlende und eine kaputte Datei melden den Fehler
    CHECK_FALSE(liesHoerschwellen("gibt_es_nicht.txt").werte.has_value());
    CHECK_FALSE(liesHoerschwellen("gibt_es_nicht.txt").fehler.empty());
    {
        const std::filesystem::path pfad = std::filesystem::temp_directory_path() / "lumiviz_test_hoerschwellen_kaputt.txt";
        std::ofstream(pfad, std::ios::binary) << "# Kopf\nt60\tfaktor\tx\t0.3\tQuelle\n";
        const HoerschwellenGelesen k = liesHoerschwellen(pfad);
        std::filesystem::remove(pfad);
        CHECK_FALSE(k.werte.has_value());
        CHECK(k.fehler.find("Zeile 2") != std::string::npos);
    }

    // Zeiten an den Grenzen 0,7923 und 1,4309 (Tolonen & Jaervelaeinen 2000, Satz 1)
    CHECK(hoerbarGleich(h, "t60_spaet_2", 0.80, 1.0));
    CHECK_FALSE(hoerbarGleich(h, "t60_spaet_2", 0.78, 1.0));
    CHECK(hoerbarGleich(h, "vorwaerts_t60", 1.43, 1.0));
    CHECK_FALSE(hoerbarGleich(h, "vorwaerts_t60", 1.44, 1.0));
    CHECK(hoerbarGleich(h, "knick_tiefe_db_2", -10.4, -10.0));
    CHECK_FALSE(hoerbarGleich(h, "knick_tiefe_db_2", -10.6, -10.0));
    CHECK_FALSE(hoerbarGleich(h, "unbekannt", 1.0, 1.0));
    // Ewert: 2 dB gleich bis 2,36, 6 bis 6,98, 10 bis 11,95; 0,5 gegen 0,8 gleich (beide unter 0,87)
    CHECK(hoerbarGleich(h, "schwebung_db_2", 2.36, 2.0));
    CHECK_FALSE(hoerbarGleich(h, "schwebung_db_2", 2.37, 2.0));
    CHECK(hoerbarGleich(h, "tiefe_db_3", 6.0, 6.98));
    CHECK_FALSE(hoerbarGleich(h, "tiefe_db_3", 6.0, 7.0));
    CHECK(hoerbarGleich(h, "tiefe_db", 11.94, 10.0));  // die Grenze liegt bei 11,945
    CHECK_FALSE(hoerbarGleich(h, "tiefe_db", 11.95, 10.0));
    CHECK(hoerbarGleich(h, "tiefe_db", 0.5, 0.8));
    CHECK_FALSE(hoerbarGleich(h, "tiefe_db", 0.5, 0.9));
    // hoerbar getroffen: r ≤ 1 oder hoerbar gleich; fehlend nie
    Abweichung a;
    a.groesse = "t60_spaet_2";
    a.skala = Skala::Faktor;
    a.modell = 0.80;
    a.ziel = 1.0;
    CHECK(hoerbarGetroffen(h, a));
    a.modell = 0.5;
    CHECK_FALSE(hoerbarGetroffen(h, a));
    a.getroffen = true;
    CHECK(hoerbarGetroffen(h, a));
    a.fehlt = true;
    a.getroffen = false;
    CHECK_FALSE(hoerbarGetroffen(h, a));
    // eindeutig ueber drei Startpunkte: in τ, oder in beide Richtungen hoerbar gleich (die engere Grenze, ln 0,2328)
    Abweichung s1 = a, s2 = a, s3 = a;
    s1.fehlt = s2.fehlt = s3.fehlt = false;
    s1.toleranz = s2.toleranz = s3.toleranz = 0.03;
    s1.modell = 1.0;
    s2.modell = 1.2;
    s3.modell = 1.26;
    CHECK(eindeutigNachHw2(h, {&s1, &s2, &s3}));
    s3.modell = 1.27;
    CHECK_FALSE(eindeutigNachHw2(h, {&s1, &s2, &s3}));
    s3.modell = 1.0;
    s3.fehlt = true;
    CHECK_FALSE(eindeutigNachHw2(h, {&s1, &s2, &s3}));
}

TEST_CASE("Loop Salamander 7.5.12 Probe: Vorzeichentest, Holm, S162 gegengerechnet (HW2 a, HW4 a)")
{
    CHECK(vorzeichenTestP(0, 15) == doctest::Approx(6.1035e-5).epsilon(1e-4));
    CHECK(vorzeichenTestP(15, 15) == doctest::Approx(6.1035e-5).epsilon(1e-4));
    CHECK(vorzeichenTestP(3, 12) == doctest::Approx(0.146).epsilon(1e-3));
    CHECK(vorzeichenTestP(6, 12) == 1.0);
    CHECK(vorzeichenTestP(0, 0) == 1.0);
    CHECK(holm({0.01, 0.04, 0.03}) == std::vector<bool>{true, false, false});
    CHECK(holm({0.04, 0.001, 0.02}) == std::vector<bool>{true, true, true});
    CHECK(holm({0.2, 0.5}) == std::vector<bool>{false, false});

    // S162, Startpunkt 1 (S164 gerechnet): hoerbar gleich 164 / 456, hoerbar getroffen nach HW2 a 215, ZF1 a 149;
    // systematisch nur t60_spaet_2 (0 von 15 zu lang) und t60_spaet_3 (1 von 14 — im Konzept steht 1 von 13, gerechnet
    // mit den Schwellen 75–140 % der Option b von HW1; nach HW1 a liegt Taste 48 mit 0,784 unter 0,7923, Befund S165)
    const HoerschwellenGelesen g = liesHoerschwellen(kHoerschwellenDatei);
    REQUIRE(g.werte.has_value());
    const std::vector<TastenSumme> s162 = liesBericht(kProbenOrdner / "S162_suche" / "ergebnis_1.txt");
    REQUIRE(s162.size() == 30);
    const HoerZahlen z = hoerZahlen(*g.werte, s162);
    CHECK(z.groessen == 456);
    CHECK(z.getroffen == 149);
    CHECK(z.hoerbarGleich == 164);
    CHECK(z.hoerbarGetroffen == 215);
    double summe = 0.0;
    for (const TastenSumme& t : s162) summe += t.summe;
    CHECK(summe == doctest::Approx(11290.1).epsilon(1e-3));
    const std::vector<GroessenBefund> b = systematik(*g.werte, s162);
    std::vector<std::string> systematisch;
    std::ostringstream text;
    for (const GroessenBefund& f : b)
    {
        if (f.systematisch) systematisch.push_back(f.groesse);
        text << f.groesse << ' ' << f.zuGross << '/' << f.nichtGetroffen << " p " << f.p << (f.systematisch ? " systematisch" : "") << "; ";
        if (f.groesse == "t60_spaet_2")
        {
            CHECK(f.zuGross == 0);
            CHECK(f.nichtGetroffen == 15);
        }
        if (f.groesse == "t60_spaet_3")
        {
            CHECK(f.zuGross == 1);
            CHECK(f.nichtGetroffen == 14);
        }
    }
    CHECK(systematisch == std::vector<std::string>{"t60_spaet_2", "t60_spaet_3"});
    MESSAGE("S162 Start 1: " << text.str());
}

namespace
{

/// Eine Suche je Taste am Ton (7.5.12, A2/A3): die Bloecke sind die Tasten, die Gruppen der finiten Differenz die
/// Kurven (`detune` ist fest und hat keine Stuetzpunkte).
struct TonSucheBloecke
{
    LmErgebnisBloecke ergebnis;
    std::vector<TastenSumme> tasten;  ///< am Ende, in der Reihenfolge der Aufgabe
    int auswertungen = 0;             ///< Toene je Ebene gezaehlt als Punkte × Tasten, mit denen der Jacobi-Matrix
    double sekunden = 0.0;
};

/// Je Iteration: die Iteration, der Stand je Taste (λ, Ende, Iterationen, Rest), die Summen je Taste am Punkt danach,
/// die Diagonale ueber alle Parameter (je Taste aus ihrer letzten Iteration) und die Dauer.
using TonBeobachterBloecke = std::function<void(const LmIterationBloecke&, const LmErgebnisBloecke&, const std::vector<TastenSumme>&,
                                                const std::vector<double>&, double)>;

TonSucheBloecke sucheJeTaste(const modelltest::SuchAufgabe& a, const LmStartBloecke& start, const LmEinstellungen& einst,
                             const TonBeobachterBloecke& beobachter = {})
{
    TonSucheBloecke s;
    const auto anfang = std::chrono::steady_clock::now();
    auto letzte = anfang;
    const LmRaum raum = modelltest::lmRaum(a.raum);
    const auto gruppen = modelltest::gruppenJeKurve(a.raum);
    std::vector<LmBlock> bloecke;
    for (const int t : a.tasten) bloecke.push_back({modelltest::parameterDerTaste(a.raum, t)});
    // je Taste die Summen der ausgewerteten Punkte, nach den Werten ihrer Parameter
    std::vector<std::map<std::vector<double>, TastenSumme>> gemerkt(bloecke.size());
    const LmResiduenBloecke amTon = [&](const std::vector<std::vector<double>>& punkte, const std::vector<std::size_t>& welche) {
        std::vector<int> tasten;
        for (const std::size_t b : welche) tasten.push_back(a.tasten[b]);
        std::vector<lumi::loop::HammerSaite> saiten;
        for (const std::vector<double>& p : punkte) saiten.push_back(modelltest::saiteAus(a.raum, p));
        const std::vector<modelltest::Auswertung> aus = modelltest::werteAus(saiten, tasten, a.strukturen, a.ziele);
        std::vector<std::vector<double>> rs;
        for (std::size_t p = 0; p < punkte.size(); ++p)
            for (std::size_t k = 0; k < welche.size(); ++k)
            {
                rs.push_back(residuen({aus[p].tasten[k]}));
                gemerkt[welche[k]][teil(punkte[p], bloecke[welche[k]].parameter)] = aus[p].tasten[k];
            }
        s.auswertungen += static_cast<int>(punkte.size() * welche.size());
        return rs;
    };
    const auto summenAm = [&](const std::vector<double>& x) {
        std::vector<TastenSumme> t;
        for (std::size_t b = 0; b < bloecke.size(); ++b) t.push_back(gemerkt[b].at(teil(x, bloecke[b].parameter)));
        return t;
    };
    LmErgebnisBloecke stand;
    stand.bloecke.resize(bloecke.size());
    for (std::size_t b = 0; b < bloecke.size(); ++b)
    {
        stand.bloecke[b].lambda = start.lambda.empty() ? einst.lambdaStart : start.lambda[b];
        stand.bloecke[b].iterationen = start.iteration.empty() ? 0 : start.iteration[b];
        stand.bloecke[b].ende = start.ende.empty() ? std::string() : start.ende[b];
    }
    std::vector<double> diagonale(a.raum.zahl(), 0.0);
    const LmBeobachterBloecke jeIteration = [&](const LmIterationBloecke& it) {
        const std::vector<TastenSumme> tasten = summenAm(it.x);
        for (std::size_t b = 0; b < bloecke.size(); ++b)
        {
            LmBlockStand& st = stand.bloecke[b];
            st.rest = tasten[b].summe;
            if (!it.bloecke[b].aktiv) continue;
            st.lambda = it.bloecke[b].lambda;
            st.ende = it.bloecke[b].ende;
            st.ohneWirkung = it.bloecke[b].ohneWirkung;
            ++st.iterationen;
        }
        stand.x = it.x;
        stand.iterationen = it.nummer;
        modelltest::diagonaleAusBloecken(bloecke, it, diagonale);
        if (beobachter) beobachter(it, stand, tasten, diagonale, sekundenSeit(letzte));
        letzte = std::chrono::steady_clock::now();
        for (std::size_t b = 0; b < bloecke.size(); ++b)
        {
            const std::vector<double> jetzt = teil(it.x, bloecke[b].parameter);
            for (auto i = gemerkt[b].begin(); i != gemerkt[b].end();) i = i->first == jetzt ? std::next(i) : gemerkt[b].erase(i);
        }
    };
    s.ergebnis = levenbergMarquardtJeBlock(amTon, bloecke, gruppen, raum.ende, start, raum, einst, jeIteration);
    s.tasten = summenAm(s.ergebnis.x);
    s.sekunden = sekundenSeit(anfang);
    return s;
}

/// Die Tasten aus `variable` („33,60“) oder alle 30.
std::vector<int> tastenAus(const char* variable)
{
    std::vector<int> tasten = modelltest::salamanderMidi();
    if (const QString t = qEnvironmentVariable(variable); !t.isEmpty())
    {
        tasten.clear();
        std::istringstream s(t.toStdString());
        std::string teilText;
        while (std::getline(s, teilText, ',')) tasten.push_back(std::stoi(teilText));
    }
    return tasten;
}

std::filesystem::path datei(const std::filesystem::path& ordner, const std::string& name, int start)
{
    return ordner / (name + "_" + std::to_string(start) + ".txt");
}

double summeG(const std::vector<modelltest::DetuneTaste>& tasten, bool nachher)
{
    double g = 0.0;
    for (const modelltest::DetuneTaste& d : tasten)
        if (d.gesetzt) g += nachher ? d.gNachher : d.gVorher;
    return g;
}

/// Die Schwebung nach A4 und am Ende je Taste als Tabelle (fuer die Ergebnisdateien).
std::string schwebungTabelle(const std::vector<modelltest::DetuneTaste>& nachA4, const std::vector<modelltest::DetuneTaste>& amEnde)
{
    std::ostringstream s;
    s << std::fixed << std::setprecision(4) << "Taste | detune Start -> nach A4 (Cent) | g vor A4 | g nach A4 | g am Ende | Ziel Hz k1-3 | Modell am Ende Hz k1-3\n";
    for (std::size_t i = 0; i < nachA4.size(); ++i)
    {
        const modelltest::DetuneTaste& d = nachA4[i];
        s << d.taste << " | " << d.vorher << " -> " << d.nachher << " | ";
        if (!d.gesetzt)
        {
            s << "nicht gesetzt (" << d.grund << ")\n";
            continue;
        }
        const modelltest::DetuneTaste& e = i < amEnde.size() ? amEnde[i] : d;
        s << d.gVorher << " | " << d.gNachher << " | " << e.gNachher << " |";
        for (const double v : d.zielHz) s << ' ' << v;
        s << " |";
        for (const double v : e.modellNachherHz) s << ' ' << v;
        s << '\n';
    }
    return s.str();
}

}  // namespace

TEST_CASE("Loop Salamander 7.5.12 Suche je Block: Datei der Iteration und Wiederaufnahme")
{
    // eine Iteration von Hand an 33 und 60: die Datei traegt λ, Ende und Iterationen je Taste und detune_fest; die
    // Wiederaufnahme liest sie zurueck (alte Dateien ohne diese Zeilen bleiben lesbar: Fall „eine Iteration an 60“)
    modelltest::SuchAufgabe a = aufgabe({33, 60}, modelltest::suchOptionen7512());
    REQUIRE(modelltest::setzeFest(a.raum, "detune", 33, 0.123456789012345));
    std::vector<LmBlock> bloecke;
    for (const int t : a.tasten) bloecke.push_back({modelltest::parameterDerTaste(a.raum, t)});
    CHECK(bloecke[0].parameter.size() + bloecke[1].parameter.size() == a.raum.zahl());
    LmIterationBloecke it;
    it.nummer = 1;
    it.x = modelltest::startpunkt(a.raum, 2);
    it.bloecke.resize(2);
    it.bloecke[0].aktiv = true;
    it.bloecke[0].lambda = 1e-4;
    it.bloecke[0].diagonale.assign(bloecke[0].parameter.size(), 4.0);
    LmErgebnisBloecke stand;
    stand.bloecke.resize(2);
    stand.bloecke[0].lambda = 1e-4;
    stand.bloecke[0].iterationen = 3;
    stand.bloecke[1].lambda = 0.1;
    stand.bloecke[1].iterationen = 2;
    stand.bloecke[1].ende = "Schritte klein";
    stand.bloecke[1].ohneWirkung = {bloecke[1].parameter.back()};
    std::vector<TastenSumme> summen(2);
    summen[0].midi = 33;
    summen[1].midi = 60;
    std::vector<double> diagonale(a.raum.zahl(), 0.0);
    modelltest::diagonaleAusBloecken(bloecke, it, diagonale);
    CHECK(diagonale[bloecke[0].parameter.front()] == 4.0);
    CHECK(diagonale[bloecke[1].parameter.front()] == 0.0);
    const std::filesystem::path ordner = std::filesystem::temp_directory_path() / "lumiviz_test_suche_7512";
    std::filesystem::remove_all(ordner);
    std::filesystem::create_directories(ordner);
    REQUIRE(modelltest::schreibeIterationBloecke(ordner, 2, it, stand, a, summen, diagonale, 1.5));
    const std::optional<modelltest::LetzteIteration> l = modelltest::liesLetzteIteration(ordner, 2);
    REQUIRE(l.has_value());
    CHECK(l->nummer == 1);
    CHECK(l->x == it.x);
    CHECK(l->lambdaJeTaste == std::vector<double>{1e-4, 0.1});
    CHECK(l->endeJeTaste == std::vector<std::string>{"", "Schritte klein"});
    CHECK(l->iterationenJeTaste == std::vector<int>{3, 2});
    CHECK(l->ende.empty());
    modelltest::Suchraum r = aufgabe({33, 60}, modelltest::suchOptionen7512()).raum;
    REQUIRE(modelltest::setzeDetuneAus(modelltest::iterationsDatei(ordner, 2, 1), r));
    CHECK(*modelltest::festWert(r, "detune", 33) == 0.123456789012345);
    CHECK(modelltest::liesX(modelltest::iterationsDatei(ordner, 2, 1)) == it.x);
    std::ifstream ein(modelltest::iterationsDatei(ordner, 2, 1));
    const std::string text((std::istreambuf_iterator<char>(ein)), std::istreambuf_iterator<char>());
    CHECK(text.find("## Ohne Wirkung (A3, kein Schritt)\n1: h 60;") != std::string::npos);
    ein.close();
    std::filesystem::remove_all(ordner);
}

TEST_CASE("Loop Salamander 7.5.12 Suche je Taste (LUMIVIZ_75_SUCHE2)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_SUCHE2");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_SUCHE2=<Ordner> sucht je Taste (A1-A5) an allen 30 Tasten (LUMIVIZ_75_SUCHE2_TASTEN=33,60) von Startpunkt 1, 2 und 3 "
                "aus (LUMIVIZ_75_SUCHE2_START=1|2|3 nur einen); laeuft Stunden");
        return;
    }
    const std::filesystem::path ordner = variable.toStdU16String();
    std::filesystem::create_directories(ordner);
    // Gegenprobe (S165, Befund des Probelaufs): ohne A4 bleibt detune frei in der Suche, sonst alles wie 7.5.12
    const bool ohneA4 = qEnvironmentVariable("LUMIVIZ_75_SUCHE2_OHNE_A4") == QStringLiteral("1");
    modelltest::SuchOptionen optionen = modelltest::suchOptionen7512();
    optionen.detuneFest = !ohneA4;
    const modelltest::SuchAufgabe basis = aufgabe(tastenAus("LUMIVIZ_75_SUCHE2_TASTEN"), optionen);
    std::vector<int> starts{1, 2, 3};
    if (const QString nur = qEnvironmentVariable("LUMIVIZ_75_SUCHE2_START"); !nur.isEmpty()) starts = {nur.toInt()};
    const HoerschwellenGelesen schwellen = liesHoerschwellen(kHoerschwellenDatei);
    INFO(schwellen.fehler);
    REQUIRE(schwellen.werte.has_value());
    const auto fortschritt = [&ordner](const std::string& zeile) {
        std::ofstream aus(ordner / "fortschritt.txt", std::ios::binary | std::ios::app);
        aus << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << '\t' << zeile << '\n';
    };
    const LmEinstellungen einst;

    // die Dauer ansagen: eine Auswertung aller Tasten gemessen; je Iteration die Gruppen und rund zwei Versuche, hoechstens
    // 20 Iterationen; A4 rund 28 Auswertungen (Raster mit Start 16, Goldener Schnitt 2 + 8, Mitte, Bericht)
    int gruppen = 0;
    for (const auto& g : modelltest::gruppenJeKurve(basis.raum)) gruppen += g.empty() ? 0 : 1;
    {
        double sekunden = 0.0;
        (void)modelltest::bewerte(basis, {modelltest::startpunkt(basis.raum, 1)}, &sekunden);
        const double schaetzung = sekunden * ((gruppen + 2.0) * einst.iterationenHoechstens + 28.0) * static_cast<double>(starts.size());
        fortschritt("Start 7.5.12, " + std::string(compilerName()) + ", " + std::to_string(modelltest::poolThreads()) + " Threads, " +
                    std::to_string(basis.tasten.size()) + " Tasten, " + std::to_string(basis.raum.zahl()) + " Parameter in " + std::to_string(gruppen) +
                    " Gruppen; eine Auswertung " + zahl(sekunden, 1) + " s, geschaetzt hoechstens " + zahl(schaetzung / 3600.0, 2) + " h");
        MESSAGE("Eine Auswertung " << sekunden << " s; die Suche dauert hoechstens rund " << schaetzung / 3600.0 << " h");
    }

    for (const int start : starts)
    {
        const auto anfang = std::chrono::steady_clock::now();
        const std::string s = std::to_string(start);
        const std::optional<modelltest::LetzteIteration> letzte = modelltest::liesLetzteIteration(ordner, start);
        if (letzte && !letzte->ende.empty() && std::filesystem::exists(datei(ordner, "ergebnis", start)))
        {
            fortschritt("Startpunkt " + s + " schon fertig");
            continue;
        }
        modelltest::SuchAufgabe a = basis;
        const std::vector<double> x0 = modelltest::startpunkt(a.raum, start);

        // A4: detune je Taste an die Schwebung, dann fest (bei der Wiederaufnahme aus der Datei)
        std::vector<modelltest::DetuneTaste> a4;
        if (const auto gelesen = modelltest::liesStufe(datei(ordner, "a4", start)); gelesen)
        {
            a4 = *gelesen;
            fortschritt("Startpunkt " + s + ": A4 aus " + datei(ordner, "a4", start).filename().string());
        }
        else
        {
            const modelltest::DetuneStufe st = modelltest::stufeDetune(a, x0);
            a4 = st.tasten;
            std::ofstream aus(datei(ordner, "a4", start), std::ios::binary);
            aus << "# A4 (7.5.12): detune je Taste an die Schwebung von Salamander, Startpunkt " << start << ", " << st.aufrufe << " Aufrufe, "
                << st.auswertungen << " Auswertungen (Saite x Taste), " << zahl(st.sekunden, 0) << " s\n"
                << modelltest::stufeText(a4);
            REQUIRE(static_cast<bool>(aus));
            int gesetzt = 0;
            for (const modelltest::DetuneTaste& d : a4) gesetzt += d.gesetzt ? 1 : 0;
            fortschritt("Startpunkt " + s + ": A4 an " + std::to_string(gesetzt) + " Tasten, Summe g " + zahl(summeG(a4, false), 4) + " -> " +
                        zahl(summeG(a4, true), 4) + ", " + zahl(st.sekunden, 0) + " s");
        }
        a.raum = modelltest::raumNachStufe(a.raum, a4);

        // A2/A3: Levenberg-Marquardt je Taste
        LmStartBloecke lmStart(x0);
        if (letzte)
        {
            lmStart.x = letzte->x;
            lmStart.nummer = letzte->nummer;
            lmStart.lambda = letzte->lambdaJeTaste;
            lmStart.ende = letzte->endeJeTaste;
            lmStart.iteration = letzte->iterationenJeTaste;
            REQUIRE(lmStart.lambda.size() == a.tasten.size());
            fortschritt("Startpunkt " + s + " nimmt Iteration " + std::to_string(letzte->nummer) + " auf");
        }
        const TonSucheBloecke t =
            sucheJeTaste(a, lmStart, einst,
                         [&](const LmIterationBloecke& it, const LmErgebnisBloecke& stand, const std::vector<TastenSumme>& tasten,
                             const std::vector<double>& diagonale, double dauer) {
                             REQUIRE(modelltest::schreibeIterationBloecke(ordner, start, it, stand, a, tasten, diagonale, dauer));
                             const Gesamt g = gesamt(tasten);
                             int aktiv = 0, angenommen = 0, ohne = 0;
                             double vorher = 0.0;
                             for (std::size_t b = 0; b < it.bloecke.size(); ++b)
                             {
                                 aktiv += it.bloecke[b].aktiv ? 1 : 0;
                                 angenommen += it.bloecke[b].angenommen ? 1 : 0;
                                 ohne += static_cast<int>(stand.bloecke[b].ohneWirkung.size());
                                 vorher += it.bloecke[b].aktiv ? it.bloecke[b].restVorher : tasten[b].summe;
                             }
                             fortschritt("Startpunkt " + s + " Iteration " + std::to_string(it.nummer) + ": Rest " + zahl(vorher, 1) + " -> " +
                                         zahl(g.summe, 1) + ", aktive Tasten " + std::to_string(aktiv) + " / " + std::to_string(it.bloecke.size()) +
                                         ", angenommen " + std::to_string(angenommen) + ", getroffen " + std::to_string(g.getroffen) + " / " +
                                         std::to_string(g.groessen) + ", ohne Wirkung " + std::to_string(ohne) + ", " + std::to_string(it.auswertungen) +
                                         " Auswertungen, " + zahl(dauer, 0) + " s");
                         });

        // die Schwebung am Ende (nur Bericht) und das Ergebnis
        const std::vector<modelltest::DetuneTaste> amEnde = modelltest::schwebungAm(a, t.ergebnis.x, a4);
        std::ofstream(datei(ordner, "a4_ende", start), std::ios::binary)
            << "# g am Ende der Suche 7.5.12, Startpunkt " << start << " (g_nachher = am Ende)\n" << modelltest::stufeText(amEnde);
        std::map<std::string, int> enden;
        std::vector<std::size_t> ohne;
        for (const LmBlockStand& b : t.ergebnis.bloecke)
        {
            ++enden[b.ende];
            ohne.insert(ohne.end(), b.ohneWirkung.begin(), b.ohneWirkung.end());
        }
        std::sort(ohne.begin(), ohne.end());
        std::ostringstream endenText;
        for (const auto& [ende, n] : enden) endenText << (endenText.tellp() > 0 ? ", " : "") << ende << ' ' << n;
        const Gesamt g = gesamt(t.tasten);
        std::ofstream aus(datei(ordner, "ergebnis", start), std::ios::binary);
        aus << "# Ergebnis der Suche 7.5.12 (V2 a: A1-A5), Startpunkt " << start << ", " << compilerName() << ", " << t.ergebnis.iterationen
            << " Iterationen, " << t.auswertungen << " Auswertungen (Punkt x Taste) in diesem Lauf, " << zahl(sekundenSeit(anfang), 0)
            << " s; Enden: " << endenText.str() << "\nrest\t" << g.summe << "\ngetroffen\t" << g.getroffen << " / " << g.groessen
            << "\nhoerbar_getroffen\t" << hoerZahlen(*schwellen.werte, t.tasten).hoerbarGetroffen << " / " << g.groessen << "\nan der Grenze\t"
            << modelltest::grenzenText(a.raum, modelltest::lmRaum(a.raum), t.ergebnis.x) << "\nohne_wirkung\t" << modelltest::ohneWirkungText(a.raum, ohne)
            << "\ng_nach_a4\t" << summeG(a4, true) << "\ng_am_ende\t" << summeG(amEnde, true) << '\n'
            << modelltest::detuneZeile(a.raum) << "\nx" << std::setprecision(17);
        for (const double v : t.ergebnis.x) aus << '\t' << v;
        aus << std::setprecision(6) << "\n\n## Je Taste: Iterationen, lambda, Ende, Stuetzpunkte ohne Wirkung\n";
        for (std::size_t b = 0; b < t.ergebnis.bloecke.size(); ++b)
        {
            const LmBlockStand& st = t.ergebnis.bloecke[b];
            aus << a.tasten[b] << '\t' << st.iterationen << '\t' << st.lambda << '\t' << st.ende << '\t' << modelltest::ohneWirkungText(a.raum, st.ohneWirkung)
                << '\n';
        }
        aus << "\n## Schwebung nach A4 und am Ende\n" << schwebungTabelle(a4, amEnde) << "\n## Kurven (wie baueVorgabe, direkt einsetzbar)\n"
            << modelltest::kurvenText(a.raum, t.ergebnis.x) << "\n## Je Taste und Groesse\n" << berichte(t.tasten, true);
        REQUIRE(static_cast<bool>(aus));
        fortschritt("Startpunkt " + s + " fertig: Rest " + zahl(g.summe, 1) + ", getroffen " + std::to_string(g.getroffen) + " / " +
                    std::to_string(g.groessen) + ", Enden " + endenText.str() + ", " + zahl(sekundenSeit(anfang), 0) + " s");
        MESSAGE("Startpunkt " << start << ": Rest " << g.summe << ", getroffen " << g.getroffen << " / " << g.groessen << ", " << endenText.str() << ", "
                              << sekundenSeit(anfang) << " s");
    }

    // nach allen verlangten Startpunkten: der beste je Taste ueber alle fertigen (A5), Vergleich und Kurzfassung
    for (const int start : starts)
        if (!std::filesystem::exists(datei(ordner, "ergebnis", start)))
        {
            MESSAGE("Startpunkt " << start << " nicht fertig — kein bester Punkt");
            return;
        }
    std::vector<int> fertig;
    std::vector<modelltest::SuchLauf> laeufe;
    std::vector<std::vector<TastenSumme>> berichteJeStart;
    for (const int start : {1, 2, 3})
    {
        if (!std::filesystem::exists(datei(ordner, "ergebnis", start))) continue;
        modelltest::SuchLauf l;
        l.raum = basis.raum;
        REQUIRE(modelltest::setzeDetuneAus(datei(ordner, "ergebnis", start), l.raum));
        l.x = modelltest::liesX(datei(ordner, "ergebnis", start));
        REQUIRE(l.x.size() == basis.raum.zahl());
        std::vector<TastenSumme> b = liesBericht(datei(ordner, "ergebnis", start));
        REQUIRE(b.size() == basis.tasten.size());
        for (const TastenSumme& ts : b) l.summeJeTaste.push_back(ts.summe);
        fertig.push_back(start);
        laeufe.push_back(std::move(l));
        berichteJeStart.push_back(std::move(b));
    }
    const modelltest::BesterJeTaste bester = modelltest::besterJeTaste(laeufe, basis.tasten);
    modelltest::SuchAufgabe ab = basis;
    ab.raum = bester.raum;
    const modelltest::Bewertet bb = modelltest::bewerte(ab, {bester.x}).front();
    {
        const Gesamt g = gesamt(bb.tasten);
        std::ofstream aus(ordner / "ergebnis_bester.txt", std::ios::binary);
        aus << "# Der beste Startpunkt je Taste (7.5.12, A5) aus den Startpunkten";
        for (const int f : fertig) aus << ' ' << f;
        aus << ", " << compilerName() << "\nrest\t" << g.summe << "\ngetroffen\t" << g.getroffen << " / " << g.groessen << "\nhoerbar_getroffen\t"
            << hoerZahlen(*schwellen.werte, bb.tasten).hoerbarGetroffen << " / " << g.groessen << "\nan der Grenze\t"
            << modelltest::grenzenText(bester.raum, modelltest::lmRaum(bester.raum), bester.x) << "\nherkunft";
        double abweichung = 0.0;
        for (std::size_t t = 0; t < basis.tasten.size(); ++t)
        {
            aus << '\t' << basis.tasten[t] << '=' << fertig[bester.herkunft[t]];
            abweichung = std::max(abweichung, std::abs(bb.tasten[t].summe - laeufe[bester.herkunft[t]].summeJeTaste[t]));
        }
        // die Summen der Quelle stehen in der Ergebnisdatei auf drei Stellen
        aus << "\nabweichung_zur_quelle\t" << abweichung << '\n' << modelltest::detuneZeile(bester.raum) << "\nx" << std::setprecision(17);
        for (const double v : bester.x) aus << '\t' << v;
        aus << std::setprecision(6) << "\n\n## Kurven (wie baueVorgabe, direkt einsetzbar)\n" << modelltest::kurvenText(bester.raum, bester.x)
            << "\n## Je Taste und Groesse\n" << berichte(bb.tasten, true);
        REQUIRE(static_cast<bool>(aus));
        CHECK(abweichung < 1e-3);
    }

    // der Vergleich der Startpunkte je Groesse (HW2 a) und je Stuetzpunkt
    int eindeutig = 0, alleGroessen = 0, tastenEindeutig = 0;
    if (fertig.size() >= 2)
    {
        std::ostringstream v;
        v << std::setprecision(4) << "# Vergleich der Startpunkte";
        for (const int f : fertig) v << ' ' << f;
        v << " (7.5.12, HW2 a: eindeutig bei einem Abstand <= max(tau, S), in beide Richtungen)\n"
          << "## Je Taste und Groesse: Taste, Groesse, Skala, Ziel, Toleranz, je Startpunkt Modell / r / getroffen, groesster Abstand in der Skala, "
             "in Toleranzen, eindeutig\n";
        for (std::size_t t = 0; t < basis.tasten.size(); ++t)
        {
            bool alle = true;
            for (std::size_t g = 0; g < berichteJeStart[0][t].groessen.size(); ++g)
            {
                std::vector<const Abweichung*> je;
                for (const auto& b : berichteJeStart) je.push_back(&b[t].groessen[g]);
                const Abweichung& a0 = *je.front();
                v << basis.tasten[t] << '\t' << a0.groesse << '\t' << skalaName(a0.skala) << '\t' << a0.ziel << '\t' << a0.toleranz;
                double klein = 0.0, gross = 0.0;
                bool fehlt = false;
                for (std::size_t i = 0; i < je.size(); ++i)
                {
                    v << '\t' << (je[i]->fehlt ? std::string("-") : zahl(je[i]->modell, 4)) << " / " << zahl(je[i]->verhaeltnis, 2) << (je[i]->getroffen ? " / ja" : " / -");
                    fehlt = fehlt || je[i]->fehlt;
                    klein = i == 0 ? je[i]->modell : std::min(klein, je[i]->modell);
                    gross = i == 0 ? je[i]->modell : std::max(gross, je[i]->modell);
                }
                const bool ja = eindeutigNachHw2(*schwellen.werte, je);
                if (fehlt)
                    v << "\tfehlt\t-";
                else
                {
                    const double d = a0.skala == Skala::Faktor ? std::log(gross / klein) : gross - klein;
                    v << '\t' << d << '\t' << (a0.toleranz > 0.0 ? d / a0.toleranz : 0.0);
                }
                v << (ja ? "\teindeutig" : "\t-") << '\n';
                eindeutig += ja ? 1 : 0;
                ++alleGroessen;
                alle = alle && ja;
            }
            tastenEindeutig += alle ? 1 : 0;
        }
        v << "\n## Je Stuetzpunkt: Kurve, Taste, Wert je Startpunkt, groesster Abstand (log: %, linear: % des Bereichs)\n";
        std::size_t j = 0;
        for (const modelltest::SuchKurve& k : basis.raum.kurven)
            for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
            {
                double klein = laeufe[0].x[j], gross = laeufe[0].x[j];
                v << k.name << '\t' << k.tasten[i];
                for (const modelltest::SuchLauf& l : laeufe)
                {
                    v << '\t' << k.wert(l.x[j]);
                    klein = std::min(klein, l.x[j]);
                    gross = std::max(gross, l.x[j]);
                }
                v << '\t' << (k.log ? 100.0 * (std::exp(gross - klein) - 1.0) : 100.0 * (gross - klein) / (k.oben[i] - k.unten[i])) << '\n';
            }
        v << "\n## detune je Taste (fest nach A4) je Startpunkt\n";
        for (const int t : basis.tasten)
        {
            v << t;
            for (const modelltest::SuchLauf& l : laeufe) v << '\t' << modelltest::festWert(l.raum, "detune", t).value_or(0.0);
            v << '\n';
        }
        v << "\neindeutig nach HW2 a: " << eindeutig << " / " << alleGroessen << " Groessen, Tasten mit allen Groessen eindeutig " << tastenEindeutig << " / "
          << basis.tasten.size() << '\n';
        std::ofstream(ordner / "vergleich.txt", std::ios::binary) << v.str();
    }

    // die Kurzfassung
    std::ostringstream k;
    k << std::fixed << std::setprecision(1) << "# Suche 7.5.12 (V2 a) — Kurzfassung, " << basis.tasten.size() << " Tasten\n";
    for (std::size_t i = 0; i < fertig.size(); ++i)
    {
        const std::optional<modelltest::LetzteIteration> l = modelltest::liesLetzteIteration(ordner, fertig[i]);
        std::map<std::string, int> enden;
        if (l)
            for (const std::string& e : l->endeJeTaste) ++enden[e.empty() ? "aktiv" : e];
        const HoerZahlen z = hoerZahlen(*schwellen.werte, berichteJeStart[i]);
        double rest = 0.0;
        for (const TastenSumme& t : berichteJeStart[i]) rest += t.summe;
        const auto a4 = modelltest::liesStufe(datei(ordner, "a4", fertig[i]));
        const auto ende = modelltest::liesStufe(datei(ordner, "a4_ende", fertig[i]));
        const modelltest::Suchraum& raum = laeufe[i].raum;
        k << "Startpunkt " << fertig[i] << ": Rest " << rest << ", getroffen (ZF1 a) " << z.getroffen << " / " << z.groessen << ", hoerbar getroffen (HW2 a) "
          << z.hoerbarGetroffen << ", " << (l ? l->nummer : 0) << " Iterationen, Enden";
        for (const auto& [e, n] : enden) k << ' ' << e << ' ' << n << ';';
        k << " an der Grenze " << anGrenzeIn(modelltest::lmRaum(raum), laeufe[i].x).size() << ", g nach A4 " << std::setprecision(3)
          << (a4 ? summeG(*a4, true) : 0.0) << ", g am Ende " << (ende ? summeG(*ende, true) : 0.0) << std::setprecision(1) << '\n';
    }
    {
        const Gesamt g = gesamt(bb.tasten);
        k << "bester je Taste: Rest " << g.summe << ", getroffen (ZF1 a) " << g.getroffen << " / " << g.groessen << ", hoerbar getroffen (HW2 a) "
          << hoerZahlen(*schwellen.werte, bb.tasten).hoerbarGetroffen << ", herkunft";
        for (std::size_t t = 0; t < basis.tasten.size(); ++t) k << ' ' << basis.tasten[t] << '=' << fertig[bester.herkunft[t]];
        k << '\n';
    }
    if (fertig.size() >= 2) k << "eindeutig nach HW2 a: " << eindeutig << " / " << alleGroessen << ", Tasten ganz eindeutig " << tastenEindeutig << '\n';
    // Stuetzpunkte ohne Wirkung je Startpunkt stehen in ergebnis_<start>.txt (Zeile ohne_wirkung)
    std::ofstream(ordner / "kurzfassung.txt", std::ios::binary) << k.str();
    MESSAGE(k.str());
}

namespace
{

/// Je Taste aus `schnitt.txt`: ob jeder ihrer Parameter glatt ist (ZF8 a (2)); leer ohne Datei.
std::map<int, bool> glattJeTaste(const std::filesystem::path& pfad)
{
    std::map<int, bool> glatt;
    std::ifstream ein(pfad, std::ios::binary);
    std::string zeile;
    int taste = 0;
    while (std::getline(ein, zeile))
    {
        if (!zeile.empty() && zeile.back() == '\r') zeile.pop_back();
        if (zeile.rfind("## Taste ", 0) == 0)
        {
            taste = std::stoi(zeile.substr(9));
            glatt[taste] = true;
        }
        else if (taste != 0 && zeile.find("| NICHT glatt") != std::string::npos)
            glatt[taste] = false;
    }
    return glatt;
}

/// Nach ZF1 a nicht erreichbar (HW3 a (3)): ein Stuetzpunkt der Taste endet an einer belegten Grenze — Lesart S165:
/// belegt sind die Grenzen mit Quelle, also `highDecay` (Bensa-Band, unten und oben) und `delta` oben (8 Cent, Tan,
/// Mori); die Untergrenze 0,01 Cent von δ ist numerisch, die uebrigen Grenzen sind ohne Quelle.
std::string belegteGrenze(const modelltest::Suchraum& raum, const std::vector<double>& x, int taste)
{
    std::size_t j = 0;
    for (const modelltest::SuchKurve& k : raum.kurven)
        for (std::size_t i = 0; i < k.tasten.size(); ++i, ++j)
        {
            if (k.tasten[i] != taste) continue;
            const double u = k.koordinate(k.unten[i]);
            const double o = k.koordinate(k.oben[i]);
            if (k.name == "highDecay" && (x[j] <= u || x[j] >= o)) return k.name + (x[j] <= u ? " unten" : " oben");
            if (k.name == "delta" && x[j] >= o) return "delta oben";
        }
    return {};
}

}  // namespace

TEST_CASE("Loop Salamander 7.5.12 Probe (LUMIVIZ_75_PROBE12)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_PROBE12");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_PROBE12=<Suchordner> urteilt nach HW3 a ueber eine fertige Suche 7.5.12 (ergebnis_*.txt, schnitt.txt) und schreibt "
                "probe.txt (rund 15 s fuer die Nullprobe der Vorgabe)");
        return;
    }
    const std::filesystem::path ordner = variable.toStdU16String();
    const HoerschwellenGelesen schwellen = liesHoerschwellen(kHoerschwellenDatei);
    INFO(schwellen.fehler);
    REQUIRE(schwellen.werte.has_value());
    const Hoerschwellen& h = *schwellen.werte;
    REQUIRE(std::filesystem::exists(ordner / "ergebnis_bester.txt"));
    std::vector<int> fertig;
    std::vector<std::vector<TastenSumme>> jeStart;
    for (const int start : {1, 2, 3})
        if (std::filesystem::exists(datei(ordner, "ergebnis", start)))
        {
            fertig.push_back(start);
            jeStart.push_back(liesBericht(datei(ordner, "ergebnis", start)));
        }
    const std::vector<TastenSumme> bester = liesBericht(ordner / "ergebnis_bester.txt");
    REQUIRE_FALSE(bester.empty());
    std::vector<int> tasten;
    for (const TastenSumme& t : bester) tasten.push_back(t.midi);
    modelltest::SuchAufgabe a = aufgabe(tasten, modelltest::suchOptionen7512());
    REQUIRE(modelltest::setzeDetuneAus(ordner / "ergebnis_bester.txt", a.raum));
    const std::vector<double> xBester = modelltest::liesX(ordner / "ergebnis_bester.txt");
    REQUIRE(xBester.size() == a.raum.zahl());

    // die Nullprobe der Vorgabe an denselben Tasten
    double sekunden = 0.0;
    const std::vector<modelltest::Auswertung> null =
        modelltest::werteAus({lumi::loop::hammerSaiteVorgabe()}, tasten, a.strukturen, a.ziele, &sekunden);
    const HoerZahlen zVorgabe = hoerZahlen(h, null.front().tasten);
    const HoerZahlen zBester = hoerZahlen(h, bester);

    std::ostringstream b;
    b << std::fixed << std::setprecision(3) << "# Probe 7.5.12 nach HW3 a (Spielweisen_Konzept §4.1), " << compilerName() << ", "
      << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\n# Suchordner " << ordner.string() << ", Startpunkte";
    for (const int f : fertig) b << ' ' << f;
    b << ", " << tasten.size() << " Tasten; Nullprobe der Vorgabe " << zahl(sekunden, 0) << " s\n\n";

    // (2) Glaette an den Probetasten
    const std::vector<int> probe{33, 48, 60, 66, 81, 84};
    const std::map<int, bool> glatt = glattJeTaste(ordner / "schnitt.txt");
    int glattTasten = 0, glattGeprueft = 0;
    for (const int t : probe)
        if (const auto it = glatt.find(t); it != glatt.end())
        {
            ++glattGeprueft;
            glattTasten += it->second ? 1 : 0;
        }
    const bool zwei = glattGeprueft == 6 && glattTasten >= 5;
    b << "(2) Glaette (alle Parameter einer Taste glatt, ohne detune): ";
    if (glatt.empty())
        b << "nicht geprueft — schnitt.txt fehlt (LUMIVIZ_75_SCHNITT mit LUMIVIZ_75_SCHNITT_X=ergebnis_bester.txt)\n";
    else
    {
        b << glattTasten << " / " << glattGeprueft << " Probetasten glatt —";
        for (const int t : probe)
            if (glatt.count(t)) b << ' ' << t << (glatt.at(t) ? " glatt" : " nicht");
        b << (zwei ? " — erfuellt (>= 5 / 6)\n" : " — verfehlt\n");
    }

    // (3) eindeutig nach HW2 a an den Probetasten ueber die Startpunkte
    int eindeutig = 0, gezaehlt = 0;
    std::ostringstream ausgenommen;
    for (const int t : probe)
    {
        const auto pos = std::find(tasten.begin(), tasten.end(), t);
        if (pos == tasten.end()) continue;
        const auto ti = static_cast<std::size_t>(pos - tasten.begin());
        if (const std::string grenze = belegteGrenze(a.raum, xBester, t); !grenze.empty())
        {
            ausgenommen << ' ' << t << " (" << grenze << ")";
            continue;
        }
        for (std::size_t g = 0; g < bester[ti].groessen.size(); ++g)
        {
            std::vector<const Abweichung*> je;
            for (const auto& s : jeStart) je.push_back(&s[ti].groessen[g]);
            eindeutig += eindeutigNachHw2(h, je) ? 1 : 0;
            ++gezaehlt;
        }
    }
    const bool drei = fertig.size() >= 2 && gezaehlt > 0 && 6 * eindeutig >= 5 * gezaehlt;
    b << "(3) eindeutig nach HW2 a ueber die Startpunkte: ";
    if (fertig.size() < 2)
        b << "nicht pruefbar — nur ein Startpunkt\n";
    else
        b << eindeutig << " / " << gezaehlt << " Groessen der Probetasten" << (drei ? " — erfuellt (>= 5/6)" : " — verfehlt") << "; ausgenommen (ZF1 a nicht erreichbar):"
          << (ausgenommen.str().empty() ? std::string(" keine") : ausgenommen.str()) << '\n';

    // (9') hoerbar getroffen am besten Punkt gegen die Vorgabe
    const bool neun = zBester.hoerbarGetroffen > zVorgabe.hoerbarGetroffen;
    b << "(9') hoerbar getroffen (HW2 a): bester Punkt " << zBester.hoerbarGetroffen << " / " << zBester.groessen << ", Vorgabe " << zVorgabe.hoerbarGetroffen
      << " / " << zVorgabe.groessen << (neun ? " — erfuellt\n" : " — verfehlt\n");
    const bool pruefbar = !glatt.empty() && fertig.size() >= 2;
    const std::string urteil = !pruefbar ? "offen (nicht alle Teile pruefbar)" : (zwei && drei && neun ? "bestanden" : "verfehlt");
    b << "\nErgebnis nach HW3 a: " << urteil << "\n\n";

    // Bericht: Treffer je Taste
    b << "## Treffer am besten Punkt: ZF1 a " << zBester.getroffen << ", hoerbar gleich allein " << zBester.hoerbarGleich << ", hoerbar getroffen (HW2 a) "
      << zBester.hoerbarGetroffen << " von " << zBester.groessen << "; Vorgabe: ZF1 a " << zVorgabe.getroffen << ", hoerbar getroffen " << zVorgabe.hoerbarGetroffen
      << "\nTaste | ZF1 a | hoerbar getroffen | alle | Vorgabe hoerbar getroffen\n";
    for (std::size_t t = 0; t < bester.size(); ++t)
    {
        const HoerZahlen zt = hoerZahlen(h, {bester[t]});
        const HoerZahlen zv = hoerZahlen(h, {null.front().tasten[t]});
        b << bester[t].midi << " | " << zt.getroffen << " | " << zt.hoerbarGetroffen << " | " << zt.groessen << " | " << zv.hoerbarGetroffen << '\n';
    }

    // Bericht: HW4 a je Groesse, Ausloeser C1
    bool c1 = false;
    b << "\n## HW4 a je Groesse (bester Punkt): nicht hoerbar getroffen mit Vorzeichen, davon Modell zu gross, p (zweiseitig, exakt), Holm alpha 0,05, "
         "fehlend, Ausreisser (|r| > 3 x Median)\n";
    for (const GroessenBefund& f : systematik(h, bester))
    {
        b << f.groesse << " | " << f.nichtGetroffen << " | " << f.zuGross << " | " << std::setprecision(5) << f.p << std::setprecision(3) << " | "
          << (f.systematisch ? "systematisch" : "-") << " | " << f.fehlend << " |";
        for (const int t : f.ausreisser) b << ' ' << t;
        b << '\n';
        c1 = c1 || (f.systematisch && (f.groesse.rfind("vorwaerts_t60", 0) == 0 || f.groesse.rfind("t60_spaet", 0) == 0));
    }
    b << "Ausloeser C1 (eine Abklinggroesse vorwaerts_t60_k oder t60_spaet_k systematisch): " << (c1 ? "ja" : "nein") << "\n";

    // Bericht: g nach A4 und am Ende je Taste, „gleich“ nach schwebung_hz (6 %)
    const Hoerschwelle* sh = h.fuer("schwebung_hz");
    b << "\n## Schwebungsfrequenz je Startpunkt: Taste | g nach A4 | g am Ende | Teiltoene mit Schwebung im Ziel gleich nach schwebung_hz (Ende)\n";
    for (const int start : fertig)
    {
        const auto a4 = modelltest::liesStufe(datei(ordner, "a4", start));
        const auto ende = modelltest::liesStufe(datei(ordner, "a4_ende", start));
        if (!a4 || !ende) continue;
        b << "Startpunkt " << start << '\n';
        for (std::size_t i = 0; i < a4->size() && i < ende->size(); ++i)
        {
            const modelltest::DetuneTaste& d = (*a4)[i];
            if (!d.gesetzt) continue;
            int gleich = 0, mit = 0;
            for (std::size_t k = 0; k < 3; ++k)
            {
                if (d.zielHz[k] <= 0.0) continue;
                ++mit;
                const double m = (*ende)[i].modellNachherHz[k];
                gleich += sh != nullptr && m > 0.0 && std::abs(std::log(m / d.zielHz[k])) <= std::min(sh->unten, sh->oben) ? 1 : 0;
            }
            b << d.taste << " | " << d.gNachher << " | " << (*ende)[i].gNachher << " | " << gleich << " / " << mit << '\n';
        }
    }
    std::ofstream aus(ordner / "probe.txt", std::ios::binary);
    aus << b.str();
    REQUIRE(static_cast<bool>(aus));
    MESSAGE(b.str());
}
