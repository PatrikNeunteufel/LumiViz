/**
 ****************************************************************************************
 * @file   test_LoopSalamanderProbe.cpp
 * @brief  Loop, Stufe 31d-7 B, 7.5.4: die Probe vor dem Setzen an den Kurven
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S154/S155: SM4 a, K1 a–K6 a,
 *         K6-1 a, SQ1 a, SQ2 a, SW1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * `LUMIVIZ_75_PROBE=<Ordner>` faehrt die Probe nach SW1 a an den Tasten 33, 48, 60, 66, 81, 84 und legt
 * `probe75.txt` in den Ordner; ohne die Variable meldet der Fall das und prueft nichts. Mit
 * `LUMIVIZ_75_PROBE_TEIL=1` nur Teil (1), die Rechenzeit (SW1 a: die Dauer wird vorher angesagt).
 * Teile (5)–(7) messen *Salamander* selbst und brauchen `LUMIVIZ_SALAMANDER=<Ordner>`; ohne ihn stehen sie
 * als „nicht gemessen“ im Bericht.
 *
 * Die Groessen je Taste und ihre Grenzen nach SQ2 a: `decay`, `lateDecay` Faktor 3 um den Messwert (ohne
 * Quelle), `highDecay` im Streuband nach Bensa et al. 2003 (L aus der Vorgabe), Verstimmung 0–6 Cent (ab
 * MIDI 90 bis 7,5), δ 0–8 Cent, γ 0–25°, u leise und laut 0–1 (ohne Quelle). Eine Auswertung rechnet die
 * Taste an den Stufen der Ebenen v6–v16 wie im Programm (K4 a), so lang wie die Kurven von *Salamander*
 * reichen und 1,5 s mehr (K5 a), misst die Kurven (7.5.2) und nimmt den Abstand je Taste (7.5.3).
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "Kurvenabstand.hpp"
#include "ModellMessung.hpp"
#include "NelderMead.hpp"

#include "audio/WavDatei.hpp"
#include "loop/Boden.hpp"
#include "loop/Fft.hpp"
#include "loop/HammerSaite.hpp"
#include "loop/HammerTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonspeicher.hpp"

#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iterator>
#include <numbers>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace lumi::messung;

namespace
{

const std::filesystem::path kWerteDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_c5.txt";
const std::filesystem::path kKurvenDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_kurven.txt";
constexpr int kProbeTasten[] = {33, 48, 60, 66, 81, 84};
constexpr int kSchnittPunkte = 15;
constexpr int kSucheHoechstens = 150;

using KurvenFeld = lumi::loop::Tastenkurve lumi::loop::HammerSaite::*;

/// Eine freie Groesse einer Taste mit ihren Grenzen (SQ2 a).
struct Groesse
{
    std::string name;
    KurvenFeld feld = nullptr;
    bool log = true;
    double unten = 0.0;
    double oben = 0.0;
    double start = 0.0;
    std::string herkunft;
};

/// `highDecay` nach Bensa et al. 2003, Gl. 37a/b, mit dem Streuband b1 × 0,5…3, b2 × 0,5…1,5 (SQ2 a); die
/// Wellenzahl π/L mit der Laenge aus der Vorgabe (Annahme: die Laengen des C6 fehlen).
std::pair<double, double> bensaBand(int midi, double laengeM)
{
    const double f0 = tastenFrequenz(midi);
    const double b1 = std::max(1e-3, 4.4e-3 * f0 - 0.04);
    const double b2 = 1.0e-6 * f0 + 1e-5;
    const double xi2 = std::pow(std::numbers::pi / laengeM, 2.0);
    const auto hd = [&](double c1, double c2) { return (c1 * b1 + c2 * b2 * xi2) / (c1 * b1 + 100.0 * c2 * b2 * xi2); };
    return {hd(0.5, 1.5), hd(3.0, 0.5)};
}

std::vector<Groesse> groessenAn(int midi, const TastenWerte& z, const lumi::loop::HammerSaite& v)
{
    using lumi::loop::HammerSaite;
    std::vector<Groesse> g;
    const int chor = lumi::loop::chorAn(v, midi);
    const double d0 = z.t60Frueh.anzahl > 0 && z.t60Frueh.wert > 0.0 ? z.t60Frueh.wert : z.t60Ausgleich.wert;
    g.push_back({"decay", &HammerSaite::abklingS, true, d0 / 3.0, 3.0 * d0, d0, "ohne Quelle, Faktor 3"});
    const auto [hu, ho] = bensaBand(midi, lumi::loop::wertAn(v.saitenLaengeM, midi, lumi::loop::KurvenSkala::Logarithmisch));
    const double h0 = std::clamp(z.highDecay.anzahl > 0 && z.highDecay.wert > 0.0 ? z.highDecay.wert : std::sqrt(hu * ho), hu, ho);
    g.push_back({"highDecay", &HammerSaite::abklingHoch, true, hu, ho, h0, "Bensa 2003"});
    const double s0 = z.zweiStufenAnteil.wert >= 0.5 && z.t60Spaet.wert > 0.0 ? std::max(z.t60Spaet.wert, d0) : 3.0 * d0;
    g.push_back({"lateDecay", &HammerSaite::abklingSpaetS, true, s0 / 3.0, 3.0 * s0, s0, "ohne Quelle, Faktor 3"});
    const double cent0 = z.schwebungCent.anzahl > 0 ? z.schwebungCent.wert : 1.0;
    if (chor >= 2)
    {
        const double centOben = midi < 90 ? 6.0 : 7.5;
        g.push_back({"Verstimmung", &HammerSaite::verstimmungCent, false, 0.0, centOben, std::clamp(cent0, 0.0, centOben), "Kirk, Cuenca, Plath"});
        g.push_back({"u leise", &HammerSaite::stossLeise, false, 0.0, 1.0, 0.25, "ohne Quelle, 0-1"});
        g.push_back({"u laut", &HammerSaite::stossLaut, false, 0.0, 1.0, 0.25, "ohne Quelle, 0-1"});
    }
    else
    {
        g.push_back({"delta", &HammerSaite::polarisationCent, false, 0.0, 8.0, std::clamp(cent0, 0.0, 8.0), "Tan 2017, Mori 2004"});
        g.push_back({"gamma", &HammerSaite::polarisationWinkelGrad, false, 0.0, 25.0, 10.0, "Tan 2017, Stegstifte"});
    }
    return g;
}

/// Der Wert einer Groesse aus der Suchkoordinate (log: ln), in ihre Grenzen.
double wertAus(const Groesse& g, double x)
{
    return std::clamp(g.log ? std::exp(x) : x, g.unten, g.oben);
}

double koordinate(const Groesse& g, double wert)
{
    return g.log ? std::log(wert) : wert;
}

lumi::loop::HammerSaite saiteMit(const lumi::loop::HammerSaite& basis, int midi, const std::vector<Groesse>& groessen,
                                 const std::vector<double>& x)
{
    lumi::loop::HammerSaite saite = basis;
    double decay = 0.0;
    for (std::size_t j = 0; j < groessen.size(); ++j)
    {
        const double v = wertAus(groessen[j], x[j]);
        (saite.*groessen[j].feld).abweichungen = {{midi, v}};
        if (groessen[j].feld == &lumi::loop::HammerSaite::abklingS) decay = v;
    }
    // die spaete Stufe ist nie kuerzer als die fruehe (HammerSaite: kuerzer gilt als Decay)
    for (std::size_t j = 0; j < groessen.size(); ++j)
        if (groessen[j].feld == &lumi::loop::HammerSaite::abklingSpaetS && saite.abklingSpaetS.abweichungen[0].wert < decay)
            saite.abklingSpaetS.abweichungen[0].wert = decay;
    return saite;
}

/// Der Abstand einer Taste aus ihren Kurven je Ebene v6–v16 (7.5.3).
struct Bewertung
{
    double gesamt = 1e9;     ///< tastenAbstand (Abklingen und Schwebung)
    double abklingen = 1e9;  ///< nur Anteil (1), gewichtet
    double schwebung = 1e9;  ///< nur Anteil (2), gewichtet
    double anteilEnde = 0.0; ///< mittlerer Anteil der letzten 20 % am Anteil (1)
    int kurven = 0;
};

Bewertung bewerte(int midi, const std::vector<TonKurven>& jeEbene, const KurvenBezug& bezug)
{
    std::vector<KurvenAbstand> abstaende;
    for (std::size_t j = 0; j < jeEbene.size(); ++j)
        for (int k = 1; k <= kKurvenTeiltoene; ++k)
            if (const auto it = bezug.find({midi, kKurvenEbeneAb + static_cast<int>(j), k}); it != bezug.end())
                abstaende.push_back(kurvenAbstand(jeEbene[j][static_cast<std::size_t>(k - 1)], it->second));
    Bewertung b;
    double w = 0.0, a1 = 0.0, a2 = 0.0, ende = 0.0;
    for (const KurvenAbstand& a : abstaende)
        if (a.gilt)
        {
            w += a.gewicht;
            a1 += a.gewicht * a.abklingenDb * a.abklingenDb;
            a2 += a.gewicht * a.schwebungDb * a.schwebungDb;
            ende += a.anteilEnde;
            ++b.kurven;
        }
    if (const std::optional<double> d = tastenAbstand(abstaende); d && w > 0.0)
    {
        b.gesamt = *d;
        b.abklingen = std::sqrt(a1 / w);
        b.schwebung = std::sqrt(a2 / w);
        b.anteilEnde = ende / b.kurven;
    }
    return b;
}

/// `einer(i)` fuer i = 0 … zahl − 1 auf `threads` Threads; jede Aufgabe rechnet fuer sich.
void parallelJe(int zahl, unsigned threads, const std::function<void(int)>& einer)
{
    std::atomic<int> naechste{0};
    std::vector<std::thread> arbeiter;
    for (unsigned a = 0; a < std::max(1u, threads); ++a)
        arbeiter.emplace_back([&] {
            for (int i = naechste++; i < zahl; i = naechste++) einer(i);
        });
    for (std::thread& t : arbeiter) t.join();
}

/// So viele Ebenen v6–v16 hat eine Auswertung.
constexpr int kAuswertungEbenen = kEbenen - kKurvenEbeneAb + 1;

/// So viele Threads rechnen die Toene (S156, `LUMIVIZ_75_ZEIT`: am schnellsten, mehr werden langsamer — der
/// Prozessor steht an der Waermegrenze); eine Auswertung so in einem Durchgang.
const unsigned kPoolThreads = std::max(1u, std::min(static_cast<unsigned>(kAuswertungEbenen), std::thread::hardware_concurrency()));

/// Je Ebene v6–v16 parallel: `einer(e)` gibt die Kurven der Ebene.
std::vector<TonKurven> jeEbene(const std::function<TonKurven(int)>& einer)
{
    std::vector<TonKurven> kurven(static_cast<std::size_t>(kAuswertungEbenen));
    parallelJe(kAuswertungEbenen, kPoolThreads, [&](int i) { kurven[static_cast<std::size_t>(i)] = einer(kKurvenEbeneAb + i); });
    return kurven;
}

/// Der Fluegel der Probe mit der Saite `saite`.
lumi::loop::Instrument fluegelMit(const lumi::loop::HammerSaite& saite)
{
    lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
    instrument.tastatur->ungedaempftAb = 89;  // D1 a
    return instrument;
}

/// Die Kurven einer Ebene des gerechneten Tons wie im Programm (K4 a), gehalten `dauerMs`.
TonKurven kurvenEbene(const lumi::loop::Instrument& instrument, int midi, int e, int dauerMs)
{
    lumi::loop::TonAngabe a;
    a.hoehe = midi;
    a.dauerMs = dauerMs;
    a.anschlagStufe = modelltest::stufeVonEbene(e);
    const lumi::loop::Klang klang = lumi::loop::rendereTon(instrument, a, modelltest::kRate);
    TonKurven k;
    (void)messeTon(klang.daten, modelltest::kRate, tastenFrequenz(midi), lumi::loop::fft(), false, &k);
    return k;
}

/// So lange reichen die Kurven von *Salamander* an der Taste, und 1,5 s mehr (K5 a).
int dauerAn(int midi, const KurvenBezug& bezug)
{
    std::size_t bis = 0;
    for (const auto& [s, z] : bezug)
        if (s.midi == midi) bis = std::max(bis, z.bereichBis);
    return static_cast<int>(static_cast<double>(bis) * Einstellungen{}.schrittS * 1000.0) + 1500;
}

/// Die Kurven des gerechneten Tons wie im Programm (K4 a), gehalten `dauerMs`, je Ebene v6–v16.
std::vector<TonKurven> kurvenTon(const lumi::loop::HammerSaite& saite, int midi, int dauerMs)
{
    const lumi::loop::Instrument instrument = fluegelMit(saite);
    return jeEbene([&](int e) { return kurvenEbene(instrument, midi, e, dauerMs); });
}

Bewertung bewerteTon(const lumi::loop::HammerSaite& saite, int midi, int dauerMs, const KurvenBezug& bezug)
{
    return bewerte(midi, kurvenTon(saite, midi, dauerMs), bezug);
}

/// Viele Auswertungen am Ton zugleich (S156): die Ebenen aller Saiten auf einem Pool — jeder Ton rechnet fuer sich,
/// die Bewertungen sind dieselben wie mit `bewerteTon` nacheinander.
std::vector<Bewertung> bewerteToene(const std::vector<lumi::loop::HammerSaite>& saiten, int midi, int dauerMs, const KurvenBezug& bezug)
{
    std::vector<lumi::loop::Instrument> instrumente;
    for (const lumi::loop::HammerSaite& s : saiten) instrumente.push_back(fluegelMit(s));
    const auto ebenen = static_cast<std::size_t>(kAuswertungEbenen);
    std::vector<TonKurven> kurven(saiten.size() * ebenen);
    parallelJe(static_cast<int>(kurven.size()), kPoolThreads, [&](int i) {
        const auto j = static_cast<std::size_t>(i);
        kurven[j] = kurvenEbene(instrumente[j / ebenen], midi, kKurvenEbeneAb + static_cast<int>(j % ebenen), dauerMs);
    });
    std::vector<Bewertung> bewertungen;
    for (std::size_t q = 0; q < saiten.size(); ++q)
    {
        const auto ab = kurven.begin() + static_cast<std::ptrdiff_t>(q * ebenen);
        bewertungen.push_back(bewerte(midi, std::vector<TonKurven>(ab, ab + static_cast<std::ptrdiff_t>(ebenen)), bezug));
    }
    return bewertungen;
}

/// Die Summe der Moden (Startpunkt nach K4 a): ohne Hammerkontakt, Laengskraft und Boden.
Bewertung bewerteModen(const lumi::loop::HammerSaite& saite, int midi, int dauerMs, const KurvenBezug& bezug)
{
    const lumi::loop::Fft& fft = lumi::loop::fft();
    return bewerte(midi,
                   jeEbene([&](int e) {
                       const std::vector<double> ton = modelltest::modenTon(saite, midi, modelltest::stufeVonEbene(e), dauerMs / 1000.0, 30);
                       std::vector<Teilton> h;
                       (void)messeKanal(ton, modelltest::kRate, tastenFrequenz(midi), fft, false, &h);
                       return kurvenAus(h, h, Einstellungen{}.schrittS);
                   }),
                   bezug);
}

/// Ein Schnitt (SW1 a (2)): hoechstens ein Minimum, kein Sprung ueber das Dreifache der mittleren Nachbaraenderung.
struct Schnitt
{
    int minima = 0;
    double groessterSchritt = 0.0;
    double mittlererSchritt = 0.0;
    [[nodiscard]] bool glatt() const { return minima <= 1 && groessterSchritt <= 3.0 * mittlererSchritt; }
};

Schnitt urteile(const std::vector<double>& d)
{
    Schnitt s;
    const std::size_t n = d.size();
    for (std::size_t i = 0; i < n; ++i)
        if ((i == 0 || d[i] < d[i - 1]) && (i + 1 == n || d[i] < d[i + 1])) ++s.minima;
    for (std::size_t i = 1; i < n; ++i)
    {
        const double schritt = std::abs(d[i] - d[i - 1]);
        s.mittlererSchritt += schritt / static_cast<double>(n - 1);
        s.groessterSchritt = std::max(s.groessterSchritt, schritt);
    }
    return s;
}

std::string zahl(double v, int stellen = 3)
{
    std::ostringstream s;
    s << std::fixed << std::setprecision(stellen) << v;
    return s.str();
}

/// Je Kurve eine Zeile: Ebene/Teilton, Abklingen, Schwebung, Gewicht, ob sie gilt, verglichene Punkte, Anfang und Ende
/// (Punkte zu 50 ms) — Modell und Salamander mit ihrem eigenen ersten und letzten Punkt.
std::string kurvenZeilen(const lumi::loop::HammerSaite& saite, int midi, int dauerMs, const KurvenBezug& bezug)
{
    std::ostringstream z;
    const std::vector<TonKurven> k = kurvenTon(saite, midi, dauerMs);
    for (std::size_t j = 0; j < k.size(); ++j)
        for (int tk = 1; tk <= kKurvenTeiltoene; ++tk)
        {
            const auto it = bezug.find({midi, kKurvenEbeneAb + static_cast<int>(j), tk});
            if (it == bezug.end()) continue;
            const TeiltonKurven& m = k[j][static_cast<std::size_t>(tk - 1)];
            const KurvenAbstand a = kurvenAbstand(m, it->second);
            z << "    v" << kKurvenEbeneAb + static_cast<int>(j) << " T" << tk << ": " << zahl(a.abklingenDb, 2) << " / " << zahl(a.schwebungDb, 2)
              << " dB, Gewicht " << zahl(a.gewicht, 2) << (a.gilt ? "" : " (gilt nicht)") << ", Punkte " << a.punkte << " (" << a.vonPunkt << "-"
              << a.endePunkt << "); Modell " << m.kurve.ersterPunkt << "-" << m.kurve.ersterPunkt + m.kurve.db.size()
              << (m.erkannt ? "" : " (nicht erkannt)") << ", Salamander " << it->second.kurve.ersterPunkt << "-"
              << it->second.kurve.ersterPunkt + it->second.kurve.db.size() << '\n';
        }
    std::string s = z.str();
    if (!s.empty()) s.pop_back();
    return s;
}

}  // namespace

/// S156, Gegenprobe zu (2): an den groessten Spruengen der Schnitte je Kurve, was sich zwischen zwei Nachbarpunkten
/// aendert — 48 `decay` 0,61 / 0,71 s und Verstimmung 4,71 / 5,14 Cent, 60 Verstimmung 0 / 0,43 Cent (die anderen Groessen am
/// Start). Nur Bericht, keine Pruefung.
TEST_CASE("Loop Salamander 31d-7 B Probe Spruenge (LUMIVIZ_75_SPRUNG)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_SPRUNG");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_SPRUNG=<Ordner> schreibt an den Spruengen der Probe den Abstand je Kurve aus");
        return;
    }
    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE_MESSAGE(gelesen.has_value(), "werte/salamander_c5.txt nicht gefunden — aus dem Projekt-Root starten");
    const std::optional<KurvenBezug> bezug = liesKurven(kKurvenDatei);
    REQUIRE(bezug.has_value());
    REQUIRE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")));
    lumi::loop::tonspeicher().leere();
    const std::vector<TastenWerte> alle = tastenWerteAus(*gelesen);
    const lumi::loop::HammerSaite basis = lumi::loop::hammerSaiteVorgabe();
    std::ofstream datei(ziel / "spruenge75.txt", std::ios::binary);

    struct Sprung
    {
        int midi;
        std::string groesse;
        int punkt;  ///< der erste der beiden Nachbarpunkte im Schnitt
    };
    for (const Sprung& s : {Sprung{48, "decay", 4}, Sprung{48, "Verstimmung", 11}, Sprung{60, "Verstimmung", 0}})
    {
        TastenWerte z;
        for (const TastenWerte& w : alle)
            if (w.midi == s.midi) z = w;
        const std::vector<Groesse> groessen = groessenAn(s.midi, z, basis);
        const int dauerMs = dauerAn(s.midi, *bezug);
        std::vector<double> x0;
        for (const Groesse& g : groessen) x0.push_back(koordinate(g, g.start));
        std::size_t j = 0;
        while (j < groessen.size() && groessen[j].name != s.groesse) ++j;
        REQUIRE(j < groessen.size());
        const Groesse& g = groessen[j];
        for (int p = s.punkt; p <= s.punkt + 1; ++p)
        {
            std::vector<double> x = x0;
            const double a = koordinate(g, g.unten), b = koordinate(g, g.oben);
            x[j] = a + (b - a) * p / (kSchnittPunkte - 1);
            const lumi::loop::HammerSaite saite = saiteMit(basis, s.midi, groessen, x);
            const Bewertung bw = bewerteTon(saite, s.midi, dauerMs, *bezug);
            datei << "MIDI " << s.midi << ' ' << g.name << ' ' << zahl(wertAus(g, x[j])) << ": Abklingen " << zahl(bw.abklingen, 2) << ", Schwebung "
                  << zahl(bw.schwebung, 2) << ", " << bw.kurven << " Kurven gelten\n"
                  << kurvenZeilen(saite, s.midi, dauerMs, *bezug) << "\n\n";
            datei.flush();
        }
    }
    lumi::loop::tonspeicher().leere();
}

TEST_CASE("Loop Salamander 31d-7 B Probe 7.5.4 (LUMIVIZ_75_PROBE)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_PROBE");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_PROBE=<Ordner> faehrt die Probe 7.5.4 nach SW1 a (LUMIVIZ_75_PROBE_TEIL=1 nur die Rechenzeit)");
        return;
    }
    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const bool nurZeit = qEnvironmentVariable("LUMIVIZ_75_PROBE_TEIL") == QStringLiteral("1");
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE_MESSAGE(gelesen.has_value(), "werte/salamander_c5.txt nicht gefunden — aus dem Projekt-Root starten");
    const std::optional<KurvenBezug> bezug = liesKurven(kKurvenDatei);
    REQUIRE_MESSAGE(bezug.has_value(), "werte/salamander_kurven.txt nicht gefunden");
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();
    const std::vector<TastenWerte> alle = tastenWerteAus(*gelesen);
    const lumi::loop::HammerSaite basis = lumi::loop::hammerSaiteVorgabe();

    std::ostringstream bericht;
    std::ofstream datei(ziel / (nurZeit ? "probe75_zeit.txt" : "probe75.txt"), std::ios::binary);
    const auto schreibe = [&](const std::string& zeile) {
        bericht << zeile << '\n';
        datei << zeile << '\n';
        datei.flush();
    };
    schreibe("Probe 7.5.4 (SW1 a) an den Tasten 33, 48, 60, 66, 81, 84 — Werte, dann je Zeile das Urteil");

    struct Taste
    {
        int midi = 0;
        TastenWerte z;
        std::vector<Groesse> groessen;
        int dauerMs = 0;
        double sekundenJeAuswertung = 0.0;
    };
    std::vector<Taste> tasten;
    for (const int midi : kProbeTasten)
    {
        Taste t;
        t.midi = midi;
        for (const TastenWerte& w : alle)
            if (w.midi == midi) t.z = w;
        t.groessen = groessenAn(midi, t.z, basis);
        t.dauerMs = dauerAn(midi, *bezug);
        std::ostringstream g;
        g << "MIDI " << midi << " (Chor " << lumi::loop::chorAn(basis, midi) << ", gehalten " << t.dauerMs << " ms):";
        for (const Groesse& gr : t.groessen)
            g << ' ' << gr.name << ' ' << zahl(gr.start) << " [" << zahl(gr.unten) << ", " << zahl(gr.oben) << "] (" << gr.herkunft << ");";
        schreibe(g.str());
        tasten.push_back(std::move(t));
    }

    // (1) Rechenzeit: eine Auswertung je Taste am Start, auf acht Threads ueber die Ebenen
    schreibe("\n(1) Rechenzeit einer Auswertung am Ton (Startwerte), Abstand dort:");
    double summeZeit = 0.0;
    for (Taste& t : tasten)
    {
        std::vector<double> x0;
        for (const Groesse& g : t.groessen) x0.push_back(koordinate(g, g.start));
        const auto start = std::chrono::steady_clock::now();
        const Bewertung b = bewerteTon(saiteMit(basis, t.midi, t.groessen, x0), t.midi, t.dauerMs, *bezug);
        t.sekundenJeAuswertung = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        summeZeit += t.sekundenJeAuswertung;
        schreibe("  MIDI " + std::to_string(t.midi) + ": " + zahl(t.sekundenJeAuswertung, 2) + " s, Abstand " + zahl(b.gesamt) + " dB (Abklingen " +
                 zahl(b.abklingen) + ", Schwebung " + zahl(b.schwebung) + ", " + std::to_string(b.kurven) + " Kurven)");
        if (nurZeit) schreibe(kurvenZeilen(saiteMit(basis, t.midi, t.groessen, x0), t.midi, t.dauerMs, *bezug));
    }
    const double mittel = summeZeit / static_cast<double>(tasten.size());
    const double setzenStunden = mittel * kSucheHoechstens * 30.0 * 2.0 / 3600.0;
    const double probeStunden = mittel * (static_cast<double>(tasten.size()) * 6.0 * kSchnittPunkte + static_cast<double>(tasten.size()) * 2.0 * kSucheHoechstens) / 3600.0;
    schreibe("  Mittel " + zahl(mittel, 2) + " s je Auswertung. Hochgerechnet (hoechstens " + std::to_string(kSucheHoechstens) +
             " Auswertungen je Suche): Setzen 30 Tasten x 2 Startpunkte " + zahl(setzenStunden, 1) + " h (Schwelle 12 h); diese Probe ohne Teil 1 hoechstens " +
             zahl(probeStunden, 1) + " h");
    if (nurZeit)
    {
        MESSAGE(bericht.str());
        return;
    }

    struct Zaehlung
    {
        std::vector<int> schnittGlattAbklingen, schnittGlattSchwebung;  // je Taste 1/0
        int eindeutig = 0, ohneGrenze = 0, modenAbweichend = 0, tasten = 0;
        double anteilEndeMittel = 0.0;
        double auswertungenMittel = 0.0;
    } ergebnis;

    for (Taste& t : tasten)
    {
        ++ergebnis.tasten;
        const std::size_t d = t.groessen.size();
        std::vector<double> x0;
        for (const Groesse& g : t.groessen) x0.push_back(koordinate(g, g.start));
        const auto beiTon = [&](const std::vector<double>& x) { return bewerteTon(saiteMit(basis, t.midi, t.groessen, x), t.midi, t.dauerMs, *bezug); };

        // (2) Schnitte je Groesse, die anderen am Start
        schreibe("\nMIDI " + std::to_string(t.midi) + " — (2) Schnitte (" + std::to_string(kSchnittPunkte) + " Punkte ueber den Bereich):");
        bool glattA = true, glattS = true;
        // alle Punkte aller Schnitte auf einem Pool (S156)
        const auto punkt = [&](std::size_t j, int p) {
            std::vector<double> x = x0;
            const double a = koordinate(t.groessen[j], t.groessen[j].unten), b = koordinate(t.groessen[j], t.groessen[j].oben);
            x[j] = a + (b - a) * p / (kSchnittPunkte - 1);
            return x;
        };
        std::vector<lumi::loop::HammerSaite> schnittSaiten;
        for (std::size_t j = 0; j < d; ++j)
            for (int p = 0; p < kSchnittPunkte; ++p) schnittSaiten.push_back(saiteMit(basis, t.midi, t.groessen, punkt(j, p)));
        const std::vector<Bewertung> schnittWerte = bewerteToene(schnittSaiten, t.midi, t.dauerMs, *bezug);
        for (std::size_t j = 0; j < d; ++j)
        {
            const Groesse& g = t.groessen[j];
            std::vector<double> dA, dS, dG;
            std::ostringstream zeile;
            zeile << "  " << g.name << ':';
            for (int p = 0; p < kSchnittPunkte; ++p)
            {
                const std::vector<double> x = punkt(j, p);
                const Bewertung& bw = schnittWerte[j * kSchnittPunkte + static_cast<std::size_t>(p)];
                dA.push_back(bw.abklingen);
                dS.push_back(bw.schwebung);
                dG.push_back(bw.gesamt);
                zeile << ' ' << zahl(wertAus(g, x[j]), 3) << '=' << zahl(bw.abklingen, 2) << '/' << zahl(bw.schwebung, 2);
            }
            const Schnitt sa = urteile(dA), ss = urteile(dS);
            glattA = glattA && sa.glatt();
            glattS = glattS && ss.glatt();
            zeile << "  | Abklingen: Minima " << sa.minima << ", groesster/mittlerer Schritt " << zahl(sa.groessterSchritt, 2) << '/'
                  << zahl(sa.mittlererSchritt, 2) << (sa.glatt() ? " glatt" : " NICHT glatt") << " | Schwebung: Minima " << ss.minima << ", "
                  << zahl(ss.groessterSchritt, 2) << '/' << zahl(ss.mittlererSchritt, 2) << (ss.glatt() ? " glatt" : " NICHT glatt");
            schreibe(zeile.str());
        }
        ergebnis.schnittGlattAbklingen.push_back(glattA ? 1 : 0);
        ergebnis.schnittGlattSchwebung.push_back(glattS ? 1 : 0);

        // (3) Startpunkt aus den Moden (K4 a, nur Anteil Abklingen), dann zwei Suchen am Ton
        std::vector<double> schritte;
        for (const Groesse& g : t.groessen) schritte.push_back(g.log ? 0.3 : 0.1 * (g.oben - g.unten));
        const NelderMead moden = nelderMead(
            [&](const std::vector<double>& x) { return bewerteModen(saiteMit(basis, t.midi, t.groessen, x), t.midi, t.dauerMs, *bezug).abklingen; },
            x0, schritte, kSucheHoechstens);
        std::vector<double> startA = moden.x;
        for (std::size_t j = 0; j < d; ++j) startA[j] = koordinate(t.groessen[j], wertAus(t.groessen[j], startA[j]));
        std::vector<double> startB = startA;
        for (std::size_t j = 0; j < d; ++j)
        {
            const Groesse& g = t.groessen[j];
            startB[j] = g.log ? startA[j] + std::log(1.6) : std::min(g.oben, std::max(1.6 * startA[j], g.unten + 0.1 * (g.oben - g.unten)));
        }
        const auto abstand = [&](const std::vector<double>& x) { return beiTon(x).gesamt; };
        // die beiden Suchen nebeneinander (S156): jede fuer sich, wie nacheinander
        NelderMead b;
        std::thread sucheB([&] { b = nelderMead(abstand, startB, schritte, kSucheHoechstens); });
        const NelderMead a = nelderMead(abstand, startA, schritte, kSucheHoechstens);
        sucheB.join();
        ergebnis.auswertungenMittel += 0.5 * (a.auswertungen + b.auswertungen) / static_cast<double>(std::size(kProbeTasten));
        bool gleich = std::abs(a.rest / b.rest - 1.0) <= 0.05;
        bool grenze = false;
        std::ostringstream z3;
        z3 << "  (3) Moden-Start (" << moden.auswertungen << " Auswertungen, Abklingen " << zahl(moden.rest) << " dB); Ton A: Rest " << zahl(a.rest)
           << " dB (" << a.auswertungen << "), Ton B: Rest " << zahl(b.rest) << " dB (" << b.auswertungen << ")\n     ";
        for (std::size_t j = 0; j < d; ++j)
        {
            const Groesse& g = t.groessen[j];
            const double va = wertAus(g, a.x[j]), vb = wertAus(g, b.x[j]);
            const bool ok = g.log ? std::abs(va / vb - 1.0) <= 0.10 : std::abs(va - vb) <= 0.10 * (g.oben - g.unten);
            gleich = gleich && ok;
            const double spanne = g.log ? std::log(g.oben / g.unten) : g.oben - g.unten;
            const double lage = g.log ? std::log(va / g.unten) : va - g.unten;
            const bool amRand = lage <= 1e-3 * spanne || lage >= (1.0 - 1e-3) * spanne;
            grenze = grenze || amRand;
            z3 << g.name << ' ' << zahl(va) << " / " << zahl(vb) << (ok ? "" : " (!)") << (amRand ? " [an der Grenze]" : "") << "; ";
        }
        ergebnis.eindeutig += gleich ? 1 : 0;
        ergebnis.ohneGrenze += grenze ? 0 : 1;
        schreibe(z3.str() + (gleich ? "\n     eindeutig" : "\n     NICHT eindeutig"));

        // (8) Moden gegen Ton am Moden-Start, voller Abstand; (K6-1 a) Anteil der letzten 20 %
        const lumi::loop::HammerSaite saiteA = saiteMit(basis, t.midi, t.groessen, startA);
        const Bewertung mo = bewerteModen(saiteA, t.midi, t.dauerMs, *bezug);
        const Bewertung to = bewerteTon(saiteMit(basis, t.midi, t.groessen, a.x), t.midi, t.dauerMs, *bezug);
        const Bewertung toStart = bewerteTon(saiteA, t.midi, t.dauerMs, *bezug);
        const double abw = std::abs(mo.gesamt / toStart.gesamt - 1.0);
        ergebnis.modenAbweichend += abw > 0.05 ? 1 : 0;
        ergebnis.anteilEndeMittel += to.anteilEnde / static_cast<double>(std::size(kProbeTasten));
        schreibe("  (8) am Moden-Start: Moden " + zahl(mo.gesamt) + " dB, Ton " + zahl(toStart.gesamt) + " dB (" + zahl(100.0 * abw, 1) +
                 " %); (K6-1 a) am Ende von Suche A: letzte 20 % tragen " + zahl(100.0 * to.anteilEnde, 1) + " % des Anteils Abklingen");
    }

    // (5)–(7) an Salamander selbst
    const QString salamander = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    int raum = 0, zweiStufig = 0, abzugWirkt = 0, abzugNegativ = 0, kurvenGesamt = 0, ebenenMehr = 0;
    std::ostringstream z567;
    if (!salamander.isEmpty())
    {
        const std::filesystem::path ordner = salamander.toStdU16String();
        const lumi::loop::Fft& fft = lumi::loop::fft();
        const double schritt = Einstellungen{}.schrittS;
        for (const int midi : kProbeTasten)
        {
            SalamanderTaste taste{midi, {}};
            for (const SalamanderTaste& st : salamanderTasten())
                if (st.midi == midi) taste = st;
            std::vector<TonKurven> gemittelt, nurL, nurR;
            for (int e = kKurvenEbeneAb; e <= kEbenen; ++e)
            {
                const std::optional<lumi::audio::WavInhalt> wav = lumi::audio::liesWavDatei(salamanderDatei(ordner, taste, e));
                REQUIRE(wav.has_value());
                auto [links, rechts] = trenne(wav->stereo);
                std::vector<double> summe(links.size());
                for (std::size_t i = 0; i < summe.size(); ++i) summe[i] = links[i] + rechts[i];
                const std::size_t ende = endeVorBlende(summe, wav->rate);
                links.resize(ende);
                rechts.resize(ende);
                const lumi::messung::Ergebnis el = miss(links, wav->rate, tastenFrequenz(midi), fft);
                const lumi::messung::Ergebnis er = miss(rechts, wav->rate, tastenFrequenz(midi), fft);
                std::vector<Teilton> hl, hr;
                for (const Teilton& t : el.teiltoene)
                    if (t.k <= kKurvenTeiltoene) hl.push_back(t);
                for (const Teilton& t : er.teiltoene)
                    if (t.k <= kKurvenTeiltoene) hr.push_back(t);
                // (5) Raum: die spaete Stufe so schnell wie der Boden?
                for (const std::vector<Teilton>* kanal : {&hl, &hr})
                    for (const Teilton& t : *kanal)
                    {
                        if (!t.abklingen.zweiStufen || t.bereichBis <= t.bereichVon) continue;
                        const auto knick = static_cast<std::size_t>(std::lround(t.abklingen.knickS / schritt));
                        if (knick + 20 >= t.bereichBis) continue;
                        ++zweiStufig;
                        std::vector<double> zeit(t.bodenDb.size());
                        for (std::size_t i = 0; i < zeit.size(); ++i) zeit[i] = static_cast<double>(i) * schritt;
                        const double boden = detail::Geraden(zeit, t.bodenDb).ueber(knick, t.bereichBis).steigung;
                        const double spaet = -60.0 / t.abklingen.t60Spaet;
                        if (boden < 0.0 && std::abs(boden / spaet - 1.0) <= 0.2) ++raum;
                    }
                // (6) Abzug des Bodens an den gemittelten Kurven
                for (int k = 1; k <= kKurvenTeiltoene; ++k)
                {
                    const auto finde = [k](const std::vector<Teilton>& v) {
                        for (const Teilton& t : v)
                            if (t.k == k) return t;
                        return Teilton{};
                    };
                    const Teilton m = kanalMittel(finde(hl), finde(hr));
                    if (m.bereichBis <= m.bereichVon) continue;
                    const Abklingkurve mit = schroederKurve(m, schritt, m.bereichVon, m.bereichBis);
                    const Abklingkurve ohne = schroederKurve(m, schritt, m.bereichVon, m.bereichBis, 0.05, false);
                    double groesste = 0.0;
                    for (std::size_t j = 0; j < std::min(mit.gueltig, ohne.gueltig); ++j) groesste = std::max(groesste, std::abs(mit.db[j] - ohne.db[j]));
                    std::size_t negativ = 0;
                    for (std::size_t i = m.bereichVon; i < m.bereichBis; ++i)
                        if (std::pow(10.0, m.pegelDb[i] / 10.0) < std::pow(10.0, m.bodenDb[i] / 10.0)) ++negativ;
                    ++kurvenGesamt;
                    abzugWirkt += groesste > 0.5 ? 1 : 0;
                    abzugNegativ += static_cast<double>(negativ) > 0.05 * static_cast<double>(m.bereichBis - m.bereichVon) ? 1 : 0;
                }
                gemittelt.push_back(kurvenAus(hl, hr, schritt));
                nurL.push_back(kurvenAus(hl, hl, schritt));
                nurR.push_back(kurvenAus(hr, hr, schritt));
            }
            // (7) Ebenen gegen Kanaele: mittlerer Abstand d = √(Abklingen² + Schwebung²)
            const auto d = [](const TeiltonKurven& a, const TeiltonKurven& b) {
                const KurvenAbstand x = kurvenAbstand(a, b);
                return x.gilt ? std::optional<double>(std::sqrt(x.abklingenDb * x.abklingenDb + x.schwebungDb * x.schwebungDb)) : std::nullopt;
            };
            double kanalSumme = 0.0, ebenenSumme = 0.0;
            int kanalZahl = 0, ebenenZahl = 0;
            for (std::size_t e = 0; e < gemittelt.size(); ++e)
                for (std::size_t k = 0; k < static_cast<std::size_t>(kKurvenTeiltoene); ++k)
                {
                    if (const auto v = d(nurL[e][k], nurR[e][k]))
                    {
                        kanalSumme += *v;
                        ++kanalZahl;
                    }
                    for (std::size_t e2 = e + 1; e2 < gemittelt.size(); ++e2)
                        if (const auto v = d(gemittelt[e2][k], gemittelt[e][k]))
                        {
                            ebenenSumme += *v;
                            ++ebenenZahl;
                        }
                }
            const double kanal = kanalZahl > 0 ? kanalSumme / kanalZahl : 0.0;
            const double ebenen = ebenenZahl > 0 ? ebenenSumme / ebenenZahl : 0.0;
            ebenenMehr += ebenen > kanal ? 1 : 0;
            z567 << "  MIDI " << midi << ": Ebenen untereinander " << zahl(ebenen, 2) << " dB, Kanaele untereinander " << zahl(kanal, 2) << " dB\n";
        }
    }

    // Urteile nach SW1 a
    const auto zaehle = [](const std::vector<int>& v) {
        int s = 0;
        for (const int x : v) s += x;
        return s;
    };
    const int n = ergebnis.tasten;
    const double setzenNeu = mittel * ergebnis.auswertungenMittel * 30.0 * 2.0 / 3600.0;
    schreibe("\nUrteile nach SW1 a:");
    schreibe(std::string("(1) Rechenzeit: ") + zahl(mittel, 2) + " s je Auswertung, im Mittel " + zahl(ergebnis.auswertungenMittel, 0) +
             " Auswertungen je Suche → Setzen " + zahl(setzenNeu, 1) + " h — " + (setzenNeu <= 12.0 ? "haelt (<= 12 h)" : "VERFEHLT → K4 b bzw. K5 b"));
    schreibe(std::string("(2) Glaette: Anteil Abklingen glatt an ") + std::to_string(zaehle(ergebnis.schnittGlattAbklingen)) + " von " + std::to_string(n) +
             ", Anteil Schwebung an " + std::to_string(zaehle(ergebnis.schnittGlattSchwebung)) + " von " + std::to_string(n) + " — " +
             (zaehle(ergebnis.schnittGlattSchwebung) >= 5 ? "Schwebung haelt" : "Schwebung VERFEHLT → K3 b") + ", " +
             (zaehle(ergebnis.schnittGlattAbklingen) >= 5 ? "Abklingen haelt" : "Abklingen VERFEHLT → SM4 b"));
    schreibe(std::string("(3) Eindeutig an ") + std::to_string(ergebnis.eindeutig) + " von " + std::to_string(n) + " — " +
             (ergebnis.eindeutig >= 5 ? "haelt" : "VERFEHLT → SM4 b"));
    schreibe(std::string("(4) Ohne Parameter an der Grenze an ") + std::to_string(ergebnis.ohneGrenze) + " von " + std::to_string(n) + " — " +
             (ergebnis.ohneGrenze >= 5 ? "haelt" : "VERFEHLT → Befund je Taste, Entscheid"));
    if (salamander.isEmpty())
        schreibe("(5)–(7) nicht gemessen (LUMIVIZ_SALAMANDER fehlt)");
    else
    {
        schreibe(std::string("(5) Raum: spaete Stufe so schnell wie der Boden (±20 %) an ") + std::to_string(raum) + " von " + std::to_string(zweiStufig) +
                 " zweistufigen Kurven — " + (3 * raum < zweiStufig ? "haelt (< 1/3)" : "VERFEHLT → SM4 b"));
        schreibe(std::string("(6) Abzug aendert > 0,5 dB an ") + std::to_string(abzugWirkt) + " von " + std::to_string(kurvenGesamt) +
                 " Kurven — " + (10 * abzugWirkt >= kurvenGesamt ? "haelt" : "VERFEHLT → K6 b") + "; mehr als 5 % Rahmen unter 0 an " +
                 std::to_string(abzugNegativ) + " — " + (10 * abzugNegativ < kurvenGesamt ? "haelt" : "VERFEHLT → K6 c"));
        schreibe("(7) Ebenen gegen Kanaele (je Taste):\n" + z567.str() + "    Ebenen unterscheiden sich mehr an " + std::to_string(ebenenMehr) + " von " +
                 std::to_string(n) + " Tasten (SW1 a nennt keine Zahl; berichtet)");
    }
    schreibe(std::string("(8) Moden gegen Ton weicht > 5 % ab an ") + std::to_string(ergebnis.modenAbweichend) + " von " + std::to_string(n) + " — " +
             (ergebnis.modenAbweichend >= 2 ? "haelt (Ton noetig)" : "VERFEHLT → K4 b"));
    schreibe(std::string("(K6-1 a) letzte 20 % tragen im Mittel ") + zahl(100.0 * ergebnis.anteilEndeMittel, 1) + " % des Anteils Abklingen — " +
             (ergebnis.anteilEndeMittel <= 0.5 ? "haelt" : "VERFEHLT → laengere Spanne"));
    MESSAGE(bericht.str());
}

/// S156: Auswertungen auf dem Pool (`bewerteToene`) geben Bit fuer Bit dieselben Bewertungen wie nacheinander
/// (`bewerteTon`) — an einer Taste mit zwei Saiten und einer mit drei, je am Start und mit anderer Verstimmung.
TEST_CASE("Loop Salamander Probe S156 Pool wie nacheinander")
{
    const std::optional<KurvenBezug> bezug = liesKurven(kKurvenDatei);
    REQUIRE_MESSAGE(bezug.has_value(), "werte/salamander_kurven.txt nicht gefunden — aus dem Projekt-Root starten");
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();
    const lumi::loop::HammerSaite basis = lumi::loop::hammerSaiteVorgabe();
    for (const int midi : {48, 84})
    {
        CAPTURE(midi);
        constexpr int kDauerMs = 2500;
        lumi::loop::HammerSaite anders = basis;
        anders.verstimmungCent.abweichungen = {{midi, 3.0}};
        const std::vector<lumi::loop::HammerSaite> saiten{basis, anders};
        const std::vector<Bewertung> pool = bewerteToene(saiten, midi, kDauerMs, *bezug);
        REQUIRE(pool.size() == saiten.size());
        for (std::size_t q = 0; q < saiten.size(); ++q)
        {
            const Bewertung einzeln = bewerteTon(saiten[q], midi, kDauerMs, *bezug);
            CHECK(einzeln.kurven > 0);
            CHECK(pool[q].kurven == einzeln.kurven);
            CHECK(pool[q].gesamt == einzeln.gesamt);
            CHECK(pool[q].abklingen == einzeln.abklingen);
            CHECK(pool[q].schwebung == einzeln.schwebung);
            CHECK(pool[q].anteilEnde == einzeln.anteilEnde);
        }
        CHECK(pool[0].gesamt != pool[1].gesamt);  // die Verstimmung wirkt: zwei verschiedene Auswertungen
    }
    lumi::loop::tonspeicher().leere();
}

/// Vor dem Pool (S156): wohin die Zeit einer Auswertung geht. Je Taste 33, 60, 84 an Ebene v11 auf einem
/// Thread: der Ton wie im Programm, darin die Hammer-Saite mit und ohne Laengskraft (Stegwinkel 0), der Rest
/// (Boden und Rahmen), die Messung; danach der Durchsatz von drei Auswertungen an Taste 60 je Zahl der Threads.
/// Nur Zeiten, keine Pruefung.
TEST_CASE("Loop Salamander 31d-7 B Probe Zeit (LUMIVIZ_75_ZEIT)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_75_ZEIT");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_75_ZEIT=<Ordner> misst die Zeit je Phase einer Auswertung und den Durchsatz je Zahl der Threads");
        return;
    }
    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::optional<KurvenBezug> bezug = liesKurven(kKurvenDatei);
    REQUIRE_MESSAGE(bezug.has_value(), "werte/salamander_kurven.txt nicht gefunden — aus dem Projekt-Root starten");
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    lumi::loop::tonspeicher().leere();
    const lumi::loop::HammerSaite basis = lumi::loop::hammerSaiteVorgabe();
    lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &basis);
    instrument.tastatur->ungedaempftAb = 89;  // D1 a
    const lumi::loop::HammerSaite& saite = instrument.schichten[0].hammerSaite;

    std::ofstream datei(ziel / "zeit75.txt", std::ios::binary);
    std::ostringstream bericht;
    const auto schreibe = [&](const std::string& zeile) {
        bericht << zeile << '\n';
        datei << zeile << '\n';
        datei.flush();
    };
    using Uhr = std::chrono::steady_clock;
    const auto sekunden = [](Uhr::time_point von) { return std::chrono::duration<double>(Uhr::now() - von).count(); };
    constexpr int kEbene = 11;
    constexpr int kDurchgaenge = 3;
    schreibe("Zeit einer Auswertung je Phase (Ebene v11, ein Thread, je " + std::to_string(kDurchgaenge) + " Durchgaenge), hardware_concurrency " +
             std::to_string(std::thread::hardware_concurrency()));

    for (const int midi : {33, 60, 84})
    {
        const int dauerMs = dauerAn(midi, *bezug);
        lumi::loop::TonAngabe angabe;
        angabe.hoehe = midi;
        angabe.dauerMs = dauerMs;
        angabe.anschlagStufe = modelltest::stufeVonEbene(kEbene);
        (void)lumi::loop::rendereTon(instrument, angabe, modelltest::kRate);  // die Antwort des Bodens liegt danach bereit

        lumi::loop::HammerAnschlag hammer;
        hammer.taste = midi;
        hammer.hz = tastenFrequenz(midi);
        hammer.stufe = angabe.anschlagStufe;
        hammer.rate = modelltest::kRate;
        hammer.gedaempft = midi < 89;
        const auto frames = static_cast<std::int64_t>(std::llround(dauerMs / 1000.0 * modelltest::kRate));
        hammer.halten = frames;
        lumi::loop::HammerSaite ohneLaengs = saite;
        ohneLaengs.stegWinkelGrad.abweichungen = {{midi, 0.0}};
        std::size_t moden = 0;
        for (const lumi::loop::TeiltonModen& t : lumi::loop::hammerTeiltonModen(saite, hammer)) moden += t.moden.size();

        for (int d = 1; d <= kDurchgaenge; ++d)
        {
            auto t0 = Uhr::now();
            const lumi::loop::Klang klang = lumi::loop::rendereTon(instrument, angabe, modelltest::kRate);
            const double ton = sekunden(t0);
            t0 = Uhr::now();
            TonKurven k;
            (void)messeTon(klang.daten, modelltest::kRate, tastenFrequenz(midi), lumi::loop::fft(), false, &k);
            const double messung = sekunden(t0);
            std::vector<float> mono;
            t0 = Uhr::now();
            lumi::loop::rechneHammerTon(saite, hammer, mono, frames);
            const double mitLaengs = sekunden(t0);
            t0 = Uhr::now();
            lumi::loop::rechneHammerTon(ohneLaengs, hammer, mono, frames);
            const double ohne = sekunden(t0);
            schreibe("  MIDI " + std::to_string(midi) + " (" + std::to_string(dauerMs) + " ms, " + std::to_string(moden) + " Moden) Durchgang " +
                     std::to_string(d) + ": Ton " + zahl(ton, 3) + " s = Hammer-Saite " + zahl(mitLaengs, 3) + " (davon Laengskraft " +
                     zahl(mitLaengs - ohne, 3) + ", ohne sie " + zahl(ohne, 3) + ") + Boden und Rest " + zahl(ton - mitLaengs, 3) +
                     "; Messung " + zahl(messung, 3) + " s; zusammen " + zahl(ton + messung, 3) + " s");
        }
    }

    // der Durchsatz: drei Auswertungen (3 × 11 Ebenen) an Taste 60 auf einem gemeinsamen Pool
    const int midi = 60;
    const int dauerMs = dauerAn(midi, *bezug);
    constexpr int kAuswertungen = 3;
    const int ebenen = kEbenen - kKurvenEbeneAb + 1;
    schreibe("\nDurchsatz an MIDI 60: " + std::to_string(kAuswertungen) + " Auswertungen zu je " + std::to_string(ebenen) +
             " Ebenen auf einem Pool (heute: 8 Threads je Auswertung, nacheinander)");
    for (const unsigned threads : {1u, 8u, 11u, 16u, 22u, 32u})
    {
        if (threads > std::thread::hardware_concurrency()) continue;
        const auto t0 = Uhr::now();
        parallelJe(kAuswertungen * ebenen, threads,
                   [&](int i) { (void)kurvenEbene(instrument, midi, kKurvenEbeneAb + i % ebenen, dauerMs); });
        const double s = sekunden(t0);
        schreibe("  " + std::to_string(threads) + " Threads: " + zahl(s, 2) + " s, " + zahl(s / kAuswertungen, 2) + " s je Auswertung");
    }
    MESSAGE(bericht.str());
}
