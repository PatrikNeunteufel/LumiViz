/**
 ****************************************************************************************
 * @file   test_InstrumentModelsStab.cpp
 * @brief  InstrumentModels, Stufe 34g: Klasse 4 (Platte und Stab) — die geteilten Bausteine Balken und Reibung
 *         (docs/audio/Instrumente_IC_Konzept.md 34g, KS3 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine.
 *
 * - Balken: der Fall aus IC `rechnung/gemeinsam/test_gemeinsam.cpp` (`bb7e6d6`) mit den IC-Grenzen — frei–frei
 *   1 : 2,757 : 5,404, f1 nach (βL)²·√(EI/ρA)/(2πL²), Knoten der ersten Mode, Kragbalken.
 * - Reibung: IC prueft sie nur ueber den gestrichenen Stab; hier die Kurve nach Galluzzo, Haften mit Hysterese und der
 *   Gleitpunkt als Loesung von x − d + (F_B/z)·μ(x) = 0.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Fft.hpp"
#include "InstrumentCore/Pool.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentMeasure/StabStimmen.hpp"
#include "InstrumentMeasure/Stimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/gemeinsam/Balken.hpp"
#include "InstrumentModels/gemeinsam/Reibung.hpp"
#include "InstrumentModels/stab/StabKern.hpp"

#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <numbers>
#include <span>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::kern;
using namespace lumi::kern::stab;

namespace
{

/// Die Bezuege aus IC (`werte/stab_ic.txt`): Abdruecke der Toene und die Fenster des Bogens aus `Stimmung.inc`.
struct IcBezug
{
    std::map<std::string, std::string> abdruck;
    std::map<std::string, Kurve> fenster;
};

const IcBezug& icBezug()
{
    static const IcBezug b = [] {
        IcBezug r;
        const std::filesystem::path pfad = std::filesystem::path(__FILE__).parent_path() / "werte" / "stab_ic.txt";
        std::ifstream f(pfad);
        std::string zeile;
        while (std::getline(f, zeile))
        {
            std::istringstream z(zeile);
            std::string art, name;
            z >> art >> name;
            if (art == "abdruck")
                z >> r.abdruck[name];
            else if (art == "fenster")
            {
                Kurve k;
                std::string paar;
                while (z >> paar)
                {
                    const auto gleich = paar.find('=');
                    k.punkte.push_back({std::stoi(paar.substr(0, gleich)), std::strtod(paar.c_str() + gleich + 1, nullptr)});
                }
                r.fenster[name] = k;
            }
        }
        return r;
    }();
    return b;
}

/// Das volle Stimmen am Modell (die Fenster des Bogens rechnen rund 150 s) nur mit `LUMIVIZ_34G_STIMMEN=1`; sonst nimmt
/// der Stimmer der Tests die Fenster aus IC (`werte/stab_ic.txt`) — gleich Bit fuer Bit, belegt mit dem vollen Lauf
/// (S178) und bei jedem Schreiben der Dateien durch `InstrumentTuneTool` (Waechter in `test_LoopStab.cpp`).
bool vollStimmen()
{
    const char* e = std::getenv("LUMIVIZ_34G_STIMMEN");
    return e != nullptr && std::string(e) == "1";
}

lumi::messung::Stimmer testStimmer()
{
    if (vollStimmen()) return lumi::messung::stabStimmer();
    return lumi::messung::Stimmer{std::string(kKernName), [](const Fabrik& f, const Werte& w, const std::vector<std::string>& namen) {
                                      const bool bogen = spielAus(w).bogen;
                                      std::vector<std::string> ohneFenster;
                                      for (const std::string& n : namen)
                                          if (!bogen || n == kPartnerPegel) ohneFenster.push_back(n);
                                      lumi::messung::GestimmteWerte aus = lumi::messung::stabStimmer().stimme(f, w, ohneFenster);
                                      if (bogen)
                                          for (const std::string& n : namen)
                                              if (n != kPartnerPegel)
                                              {
                                                  Kurve k = icBezug().fenster.at(n);
                                                  k.belegkraft = Belegkraft::Rechnung;
                                                  k.quellen = {"lumiviz"};
                                                  aus.emplace_back(n, k);
                                              }
                                      return aus;
                                  }};
}

/// Die Werte einer Vorlage, gestimmt und mit Bezug — wie `InstrumentTuneTool --all --reference` (einmal je Lauf).
const Werte& fertigeWerte(const std::string& name)
{
    static std::mutex sperre;
    static std::map<std::string, Werte> fertig;
    const std::lock_guard<std::mutex> g(sperre);
    if (const auto it = fertig.find(name); it != fertig.end()) return it->second;
    for (const Vorlage& v : vorlagen())
        if (v.name == name)
        {
            Werte w = werteAus(v.spiel);
            const lumi::messung::StimmBericht b = lumi::messung::stimmeUndNormiere(kerne(), {testStimmer()}, kKernName, w, true, true);
            REQUIRE_MESSAGE(b.fehler.empty(), name << ": " << (b.fehler.empty() ? std::string() : b.fehler.front()));
            const std::vector<std::string> f = richteWerte(kernTyp().tafel, w);
            REQUIRE_MESSAGE(f.empty(), name << ": " << (f.empty() ? std::string() : f.front()));
            return fertig.emplace(name, std::move(w)).first->second;
        }
    FAIL("keine Vorlage " << name);
    static const Werte leer;
    return leer;
}

/// Das fertige Spiel unter dem IC-Namen (Marimba, Xylophon …).
StabSpiel instrument(const std::string& icName)
{
    for (const Vorlage& v : vorlagen())
        if (v.spiel.name == icName) return spielAus(fertigeWerte(v.name));
    FAIL("kein Instrument " << icName);
    return {};
}

StabAnschlag anschlag(int note, int stufe)
{
    StabAnschlag a;
    a.note = note;
    a.hz = 440.0 * std::exp2((note - 69) / 12.0);
    a.stufe = stufe;
    return a;
}

double rms(std::span<const float> werte)
{
    if (werte.empty()) return 0.0;
    double s = 0.0;
    for (const float w : werte) s += static_cast<double>(w) * w;
    return std::sqrt(s / static_cast<double>(werte.size()));
}

double dB(double x)
{
    return x > 0.0 ? 20.0 * std::log10(x) : -300.0;
}

double spitze(const std::vector<float>& t)
{
    double s = 0.0;
    for (const float x : t) s = std::max(s, static_cast<double>(std::abs(x)));
    return s;
}

/// Der Betrag der DFT bei f ueber einen Ausschnitt (Hann-Fenster) — wie IC `test_stab.cpp`.
double bandBetrag(const std::vector<float>& x, std::size_t ab, std::size_t n, double f, double fs)
{
    Komplex s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < x.size(); ++i)
    {
        const double w = 0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * i / n);
        s += w * x[ab + i] * std::exp(Komplex(0.0, -2.0 * std::numbers::pi * f * (ab + i) / fs));
    }
    return std::abs(s);
}

/// Der Ton, den IC je Instrument abdrueckt: mittlere Note bei Stufe −12, das Vibraphon mit dem Motor auf 3 U/s, so lange
/// wie `stabNachklang` (hoechstens 3 s).
std::vector<float> mitteTon(const StabSpiel& s)
{
    StabAnschlag n = anschlag((s.noteMin + s.noteMax) / 2, -12);
    if (s.motor) n.motor = {{0.0, 3.0}};
    std::vector<float> aus;
    rechneStabTon(s, n, aus, stabNachklang(s, n, 48000 * 3));
    return aus;
}

StabAnschlag gestrichen(int note, double e)
{
    StabAnschlag a;
    a.note = note;
    a.hz = 440.0 * std::exp2((note - 69) / 12.0);
    a.gestrichen = true;
    a.expression = e;
    a.haltenS = 1.0;
    return a;
}

}  // namespace

TEST_CASE("InstrumentModels 34g Balken: frei–frei 1 : 2,757 : 5,404, f1 nach (βL)²·√(EI/ρA)/(2πL²), Kragbalken")
{
    const std::vector<double> v = freiFreiVerhaeltnisse(4);
    CHECK(v[1] == doctest::Approx(2.7565).epsilon(1e-4));
    CHECK(v[2] == doctest::Approx(5.4039).epsilon(1e-4));
    Balken b;
    b.laengeM = 0.5;
    b.eModulPa = 7e10;
    b.dichteKgM3 = 2700.0;
    b.elemente = 80;
    const std::vector<BalkenMode> m = biegeModen(b, 4);
    REQUIRE(m.size() == 4);
    const double ei = 7e10 * 0.05 * std::pow(0.02, 3) / 12.0;
    const double ra = 2700.0 * 0.05 * 0.02;
    const double f1 = 4.730040745 * 4.730040745 / (2.0 * std::numbers::pi * 0.25) * std::sqrt(ei / ra);
    CHECK(m[0].hz == doctest::Approx(f1).epsilon(1e-4));
    for (int i = 1; i < 4; ++i) CHECK(m[static_cast<std::size_t>(i)].hz / m[0].hz == doctest::Approx(v[static_cast<std::size_t>(i)]).epsilon(1e-3));
    // Knotenlinie der ersten Mode bei 0,2242·L (Lehrbuch)
    double knoten = 0.0;
    for (int i = 1; i <= 40; ++i)
        if (formAn(m[0], b, (i - 1) * 0.25 / 40) * formAn(m[0], b, i * 0.25 / 40) <= 0.0) knoten = (i - 0.5) * 0.25 / 40;
    CHECK(knoten / 0.5 == doctest::Approx(0.2242).epsilon(0.02));
    b.links = Lager::Eingespannt;
    const std::vector<BalkenMode> k = biegeModen(b, 3);
    const std::vector<double> beta = kragBetaL(3);
    CHECK(beta[0] == doctest::Approx(1.8751).epsilon(1e-4));
    for (int i = 0; i < 3; ++i)
        CHECK(k[static_cast<std::size_t>(i)].hz ==
              doctest::Approx(beta[static_cast<std::size_t>(i)] * beta[static_cast<std::size_t>(i)] / (2.0 * std::numbers::pi * 0.25) * std::sqrt(ei / ra)).epsilon(1e-3));
}

TEST_CASE("InstrumentModels 34g Reibung: Galluzzo, Haften mit Hysterese, Gleitpunkt")
{
    // μ(0) = 0,4 + 0,45 + 0,35, fuer grosses |Δv| die Gleitreibung 0,35; dazwischen fallend
    CHECK(reibwertGalluzzo(0.0) == doctest::Approx(1.2).epsilon(1e-15));
    CHECK(reibwertGalluzzo(10.0) == doctest::Approx(0.35).epsilon(1e-12));
    CHECK(reibwertGalluzzo(0.01) > reibwertGalluzzo(0.1));

    // ohne Kraft keine Reibung, und das Haften endet
    bool haftet = true;
    double letzte = -1.0;
    CHECK(reibung(0.1, 0.2, 0.0, 10.0, 1.2, haftet, letzte) == 0.0);
    CHECK_FALSE(haftet);

    // kleiner Unterschied, grosse Kraft: es haftet — die Kraft haelt v = v_B
    const double z = 10.0;
    haftet = false;
    letzte = -1.0;
    const double fh = reibung(0.199, 0.2, 1.0, z, 1.2, haftet, letzte);
    CHECK(haftet);
    CHECK(fh == doctest::Approx(z * (0.2 - 0.199)).epsilon(1e-15));
    // Hysterese: haftet es, bleibt es haften, solange die Haftkraft reicht
    const double fh2 = reibung(0.15, 0.2, 1.0, z, 1.2, haftet, letzte);
    CHECK(haftet);
    CHECK(fh2 == doctest::Approx(z * (0.2 - 0.15)).epsilon(1e-15));

    // grosser Unterschied: es gleitet; |Δv| = x loest x − d + (F_B/z)·μ(x) = 0, die Kraft F_B·μ(x) gegen v_h − v_B
    haftet = false;
    letzte = -1.0;
    const double vh = 0.8, vb = 0.2, fb = 0.5;
    const double f = reibung(vh, vb, fb, z, 1.2, haftet, letzte);
    CHECK_FALSE(haftet);
    REQUIRE(letzte >= 0.0);
    const double d = std::abs(vh - vb);
    // die Halbierung des ersten Aufrufs (30 Schritte auf d/16) trifft den Schnitt auf rund 3e-11
    CHECK(std::abs(letzte - d + fb / z * reibwertGalluzzo(letzte)) < 1e-10);
    CHECK(f == doctest::Approx(-fb * reibwertGalluzzo(letzte)).epsilon(1e-15));
    CHECK(f < 0.0);
    // Newton ab dem letzten Gleitpunkt trifft denselben Punkt — genauer als die Halbierung des ersten Aufrufs
    // (30 Schritte auf d/16: rund 3e-11), darum die Grenze 1e-9
    const double vorher = letzte;
    const double f2 = reibung(vh, vb, fb, z, 1.2, haftet, letzte);
    CHECK(letzte == doctest::Approx(vorher).epsilon(1e-9));
    CHECK(letzte - d + fb / z * reibwertGalluzzo(letzte) == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(f2 == doctest::Approx(f).epsilon(1e-9));
}

// --- 34g-3: der Kern `bar` --------------------------------------------------------------------------------------

TEST_CASE("InstrumentModels 34g Kern bar: Tafel, Fabrik, Vorlagen, Werte hin und zurueck")
{
    const KernTyp& t = kernTyp();
    const std::vector<std::string> fehler = pruefeTafel(t.tafel);
    CHECK_MESSAGE(fehler.empty(), (fehler.empty() ? std::string() : fehler.front()));
    REQUIRE(kerne().typ(kKernName) != nullptr);
    CHECK(kerne().namen() == std::vector<std::string>{"hammerString", "bar", "membrane"});
    CHECK(t.tafel.bezugWert == kBezug);
    CHECK(t.tafel.umfangVon == "noteMin");
    CHECK(t.auskunft.rechenweg == Rechenweg::Note);
    // die gestimmten Werte: der Pegel des Partners vor den Fenstern (KS2 a)
    std::vector<std::string> gestimmt;
    for (const ParameterAngabe& a : t.tafel.werte)
        if (a.gestimmt) gestimmt.push_back(a.name);
    CHECK(gestimmt == std::vector<std::string>{"partnerGain", "bowForceLowPp", "bowForceHighPp", "bowForceLowMf", "bowForceHighMf",
                                               "bowForceLowFf", "bowForceHighFf"});

    // KS4 a: die Namen; die Steeldrum geht nicht mit
    std::vector<std::string> namen, mit;
    for (const Vorlage& v : vorlagen())
    {
        namen.push_back(v.name);
        if (v.ausgeliefert) mit.push_back(v.name);
    }
    CHECK(namen.size() == 10);
    CHECK(mit == std::vector<std::string>{"marimba", "xylophone", "vibraphone", "glockenspiel", "tubular_bells", "celesta", "kalimba",
                                          "tine_electric_piano", "reed_electric_piano"});
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        // hin und zurueck Bit fuer Bit (Werte → Spiel → Werte), an gerichteten Werten: der Wurlitzer erbt die
        // konstanten Kurven des Rhodes mit Punkten bei 28 und 100, gerichtet stehen sie auf seinem Umfang 33 … 96
        // (die Werte gleich, der Ton auch — der Abdruck im naechsten Fall)
        const Werte w = werteAus(v.spiel);
        Werte gerichtet = fertigeWerte(v.name);
        gerichtet.gestimmtAus.clear();
        Werte zurueck = werteAus(spielAus(gerichtet));
        CHECK(richteWerte(t.tafel, zurueck).empty());  // in die Reihenfolge der Tafel
        CHECK(zurueck == gerichtet);
        for (const auto& [n, wert] : gerichtet.felder)
            if (const Wert* z = zurueck.finde(n); z == nullptr || !(*z == wert)) CHECK_MESSAGE(false, "anders: " << n << " " << wertAlsText(wert) << " / " << (z ? wertAlsText(*z) : std::string("-")));
        // der Umfang des Instruments (KS1 a)
        CHECK(tastenumfang(t.tafel, w) == std::pair<int, int>{v.spiel.noteMin, v.spiel.noteMax});
        // jede Kurve traegt eine Quelle (WZ1 a)
        for (const auto& [n, wert] : w.felder)
            if (const Kurve* k = std::get_if<Kurve>(&wert); k != nullptr && !k->punkte.empty()) CHECK_MESSAGE(!k->quellen.empty(), n);
    }
}

TEST_CASE("InstrumentModels 34g Kern bar: gestimmt und normiert Bit fuer Bit wie IC (11 Abdruecke, Fenster aus Stimmung.inc)")
{
    const IcBezug& ic = icBezug();
    REQUIRE_MESSAGE(ic.abdruck.size() == 11, "werte/stab_ic.txt fehlt oder ist unvollstaendig");
    REQUIRE(ic.fenster.size() == 6);
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const StabSpiel s = spielAus(fertigeWerte(v.name));
        const std::vector<float> ton = mitteTon(s);
        CHECK(spitze(ton) > 0.0);
        CHECK(std::isfinite(rms(ton)));
        CHECK(abdruck(ton) == ic.abdruck.at("stab_" + v.spiel.name + "_mitte_a-12"));
    }
    // die Fenster des Bogens, am Modell gestimmt (nur mit LUMIVIZ_34G_STIMMEN=1, sonst die aus IC), gegen die aus IC
    if (!vollStimmen()) MESSAGE("die Fenster des Bogens aus IC — am Modell gestimmt nur mit LUMIVIZ_34G_STIMMEN=1 (rund 150 s)");
    const Werte& vib = fertigeWerte("vibraphone");
    for (const auto& [name, kurve] : ic.fenster)
    {
        CAPTURE(name);
        const Kurve& k = vib.kurve(name);
        CHECK(k.punkte == kurve.punkte);
        CHECK(k.belegkraft == Belegkraft::Rechnung);
    }
    // der gestrichene Ton mit den Fenstern aus IC (unabhaengig davon, ob die eigenen gleich sind)
    StabSpiel mitIc = spielAus(vib);
    for (int i = 0; i < 3; ++i)
    {
        mitIc.bogenKraftUnten[static_cast<std::size_t>(i)] = ic.fenster.at(std::string(kBogenUnten[i]));
        mitIc.bogenKraftOben[static_cast<std::size_t>(i)] = ic.fenster.at(std::string(kBogenOben[i]));
    }
    std::vector<float> g;
    rechneStabTon(mitIc, gestrichen(72, 0.5), g, 48000);
    CHECK(abdruck(g) == ic.abdruck.at("stab_Vibraphon_gestrichen_m72_mf"));
    rechneStabTon(spielAus(vib), gestrichen(72, 0.5), g, 48000);
    CHECK(abdruck(g) == ic.abdruck.at("stab_Vibraphon_gestrichen_m72_mf"));
}

TEST_CASE("InstrumentModels 34g Stab: Moden aus den Kurven — 1 : 4 : 10 (Marimba), 1 : 3 : 9 (Xylophon), darueber der Balken")
{
    const StabModen m = stabModen(instrument("Marimba"), anschlag(60, 0));
    REQUIRE(m.stab.size() >= 4);
    CHECK(m.stab[1].hz / m.stab[0].hz == doctest::Approx(4.0));
    CHECK(m.stab[2].hz / m.stab[0].hz == doctest::Approx(10.0));
    const std::vector<double> v = freiFreiVerhaeltnisse(4);
    CHECK(m.stab[3].hz / m.stab[2].hz == doctest::Approx(v[3] / v[2]).epsilon(1e-3));
    const StabModen x = stabModen(instrument("Xylophon"), anschlag(72, 0));
    CHECK(x.stab[1].hz / x.stab[0].hz == doctest::Approx(3.0));
    CHECK(x.stab[2].hz / x.stab[0].hz == doctest::Approx(9.0));
    // Mitte angeschlagen: die antisymmetrische zweite Mode bekommt nichts (La Favre, nur Bericht; hier Rechnung)
    CHECK(std::abs(m.stab[1].form) < 1e-6 * std::abs(m.stab[0].form));
    // das geschlossene Rohr: Partnermoden bei 1, 3, 5 … — beim Xylophon trifft 3 die zweite Stabmode (1 : 3 : 9)
    REQUIRE(x.partner.size() >= 2);
    CHECK(x.stab[0].partner == 0);
    CHECK(x.stab[1].partner == 1);
    CHECK(x.stab[2].partner == 4);  // 9·f_r
    // bei der Marimba haben nur Mode 1 (1) und keine weitere einen Partner (4 und 10 sind gerade)
    CHECK(m.stab[0].partner == 0);
    CHECK(m.stab[1].partner == -1);
}

TEST_CASE("InstrumentModels 34g Stab gegen Rucz & Ulveczki 2021: T60 ohne Rohr „close to 10 s“, mit Rohr 0,7 s, +15 dB, Kontakt 0,6 ms")
{
    const StabSpiel s = instrument("Xylophon");
    StabAnschlag a = anschlag(65, -6);
    a.hz = 348.8;  // der Stab der Arbeit
    const StabModen m = stabModen(s, a);
    CHECK(m.stab[0].t60S == doctest::Approx(kLn1000 / (2.57e-4 * 2 * std::numbers::pi * 348.8)).epsilon(2e-3));
    CHECK(m.stab[0].t60S > 9.0);
    CHECK(m.stab[0].t60S < 13.0);
    const double sigmaPaar = 0.5 * (sigmaAusT60(m.stab[0].t60S) + sigmaAusT60(m.partner[0].t60S));
    CHECK(kLn1000 / sigmaPaar == doctest::Approx(0.7).epsilon(0.03));
    std::vector<float> ton;
    rechneStabTon(s, a, ton, 48000);
    const double f0 = m.stab[0].hz;
    const double abfall = dB(bandBetrag(ton, 9600, 4800, f0, 48000.0) / bandBetrag(ton, 28800, 4800, f0, 48000.0));
    CHECK(abfall == doctest::Approx(0.4 * 60.0 / (kLn1000 / sigmaPaar)).epsilon(0.15));
    // +15 dB (gestimmt, KS2 a): mit Rohr gegen ohne, Effektivwert der ersten 0,5 s
    StabSpiel ohne = s;
    ohne.partner = PartnerArt::Keiner;
    std::vector<float> t1, t2;
    rechneStabTon(s, a, t1, 24000);
    rechneStabTon(ohne, a, t2, 24000);
    CHECK(dB(rms(t1) / rms(t2)) == doctest::Approx(15.0).epsilon(0.01));
    StabSpiel eins = s;
    eins.tempoMax = 1.0;
    StabMessung messung;
    StabAnschlag b = a;
    b.stufe = 0;
    rechneStabTon(eins, b, ton, 4800, &messung);
    CHECK(messung.kontaktS == doctest::Approx(0.6e-3).epsilon(0.15));
    CHECK(messung.kontakte == 1);
}

TEST_CASE("InstrumentModels 34g Stab: Konvergenz des Kontakts, Rhodes-Tine nach Falaize & Hélie, Grenzen (L11)")
{
    StabSpiel s = instrument("Marimba");
    std::vector<float> a, b;
    StabMessung ma, mb;
    rechneStabTon(s, anschlag(60, 0), a, 4800, &ma);
    s.ueberabtastung = 256;
    rechneStabTon(s, anschlag(60, 0), b, 4800, &mb);
    CHECK(ma.kraftSpitzeN == doctest::Approx(mb.kraftSpitzeN).epsilon(0.01));
    CHECK(rms(a) == doctest::Approx(rms(b)).epsilon(0.01));

    Balken t;
    t.laengeM = 0.0783;
    t.eModulPa = 180e9;
    t.dichteKgM3 = 7750.0;
    t.hoeheM = [](double) { return 0.002 * std::sqrt(3.0); };
    t.links = Lager::Eingespannt;
    t.elemente = 60;
    CHECK(biegeModen(t, 2)[0].hz == doctest::Approx(440.0).epsilon(0.005));

    StabSpiel g = instrument("Marimba");
    const StabSpiel v = g;
    g.verhaeltnis2 = kurveAusPunkten({{60, 100.0}});
    g.kopplungJeKritisch = kurveAusPunkten({{60, 1.0}});
    g.t60S = Kurve{};
    g.ueberabtastung = 0;
    richteStabSpiel(g, v);
    CHECK(wertAn(g.verhaeltnis2, 60, Skala::Linear) == 20.0);
    CHECK(wertAn(g.kopplungJeKritisch, 60, Skala::Linear) == 1.05);
    CHECK(g.t60S == v.t60S);
    CHECK(g.ueberabtastung == 1);
}

TEST_CASE("InstrumentModels 34g Stab: Motor, Daempferleiste, Tonabnehmer, Helligkeit")
{
    const StabSpiel v = instrument("Vibraphon");
    // der Motor moduliert den Grundton mit der doppelten Drehzahl
    StabAnschlag a = anschlag(65, -6);
    a.motor = {{-1.0, 4.0}};
    std::vector<float> ton;
    rechneStabTon(v, a, ton, 96000);
    std::vector<double> huelle;
    for (std::size_t i = 4800; i + 240 < ton.size(); i += 240)
    {
        double s = 0.0;
        for (std::size_t k = 0; k < 240; ++k) s += std::abs(ton[i + k]);
        huelle.push_back(s);
    }
    const auto amp = [&](double f) {
        Komplex s(0.0, 0.0);
        double mittel = 0.0;
        for (const double h : huelle) mittel += h;
        mittel /= static_cast<double>(huelle.size());
        for (std::size_t i = 0; i < huelle.size(); ++i) s += (huelle[i] - mittel) * std::exp(Komplex(0.0, -2.0 * std::numbers::pi * f * i * 240.0 / 48000.0));
        return std::abs(s);
    };
    CHECK(amp(8.0) > 3.0 * amp(5.0));
    CHECK(amp(8.0) > 3.0 * amp(12.0));
    std::vector<float> ruhig;
    rechneStabTon(v, anschlag(65, -6), ruhig, 96000);
    CHECK(ruhig != ton);

    // die Daempferleiste dampft ab der Stelle, das halbe Pedal halb so stark
    StabAnschlag d = anschlag(69, -6);
    d.daempferAb = 24000;
    std::vector<float> voll, halb, frei;
    rechneStabTon(v, d, voll, 72000);
    d.andruck = 0.5;
    rechneStabTon(v, d, halb, 72000);
    d.daempferAb = -1;
    rechneStabTon(v, d, frei, 72000);
    const auto spaet = [](const std::vector<float>& t) { return rms(std::span<const float>(t).subspan(36000, 12000)); };
    CHECK(spaet(voll) < spaet(halb));
    CHECK(spaet(halb) < spaet(frei));
    CHECK(dB(spaet(frei) / spaet(voll)) > 20.0);

    // der Tonabnehmer des Rhodes klirrt mit der Staerke
    const StabSpiel r = instrument("Rhodes");
    const auto klirr = [&](int stufe) {
        std::vector<float> t;
        const StabAnschlag k = anschlag(57, stufe);
        rechneStabTon(r, k, t, 24000);
        return bandBetrag(t, 4800, 9600, 2 * k.hz, 48000.0) / bandBetrag(t, 4800, 9600, k.hz, 48000.0);
    };
    CHECK(klirr(0) > 1.5 * klirr(-30));

    // die Helligkeit steigt mit der Staerke (kuerzerer Kontakt)
    const auto schwerpunkt = [](const std::vector<float>& t) {
        std::vector<double> x(8192, 0.0);
        for (std::size_t i = 0; i < x.size() && i < t.size(); ++i) x[i] = t[i];
        std::vector<std::complex<double>> sp(x.size() / 2 + 1);
        fft().vorwaerts(x, sp);
        double a1 = 0.0, b1 = 0.0;
        for (std::size_t k = 1; k < sp.size(); ++k)
        {
            a1 += k * std::norm(sp[k]);
            b1 += std::norm(sp[k]);
        }
        return a1 / b1 * 48000.0 / 8192.0;
    };
    for (const char* name : {"Marimba", "Xylophon", "Glockenspiel"})
    {
        CAPTURE(name);
        const StabSpiel s = instrument(name);
        std::vector<float> leise, laut;
        rechneStabTon(s, anschlag(s.noteMin + 12, -30), leise, 8192);
        rechneStabTon(s, anschlag(s.noteMin + 12, 0), laut, 8192);
        CHECK(schwerpunkt(laut) > schwerpunkt(leise));
    }
}

TEST_CASE("InstrumentModels 34g Vibraphon gestrichen (KS3 a): obere Lage bei pp und mf auf ±10 Cent; C4 ohne Resonator auch")
{
    const StabSpiel v = instrument("Vibraphon");
    REQUIRE(v.bogen);
    int getroffen = 0, gesamt = 0;
    for (int note = 63; note <= v.noteMax; note += 3)
        for (const double e : {0.0, 0.5})
        {
            const StabAnschlag a = gestrichen(note, e);
            std::vector<float> t;
            StabMessung m;
            rechneStabTon(v, a, t, 48000, &m);
            ++gesamt;
            const double hz = lumi::messung::stabSpielHz(m, a.rate);
            const double c = hz > 0.0 ? 1200.0 * std::log2(hz / a.hz) : 1e9;
            if (std::abs(c) < 10.0 && m.haftAnteil > 0.05) ++getroffen;
        }
    MESSAGE("Vibraphon gestrichen E♭4 … F6, pp und mf: " << getroffen << " von " << gesamt << " auf ±10 Cent");
    CHECK(getroffen * 10 >= gesamt * 9);
    // ohne Resonator spielt auch die tiefe Lage (mit ihm rastet sie auf die obere Mode der Kopplung ein, IC offen.md)
    StabSpiel ohne = v;
    ohne.partner = PartnerArt::Keiner;
    StabAnschlag a = gestrichen(60, 0.5);
    a.bogenKraftN = 1.6;
    a.bogenVbMS = 0.18;
    std::vector<float> t;
    StabMessung m;
    rechneStabTon(ohne, a, t, 48000, &m);
    const double hz = lumi::messung::stabSpielHz(m, a.rate);
    REQUIRE(hz > 0.0);
    CHECK(std::abs(1200.0 * std::log2(hz / a.hz)) < 10.0);
    CHECK(m.haftAnteil > 0.05);
}

TEST_CASE("InstrumentModels 34g Kern bar: der Adapter — Daempfer am Ende der Note, Spuren, gestrichen ab bowedFrom, Nachklang")
{
    const Werte& w = fertigeWerte("vibraphone");
    const std::shared_ptr<const Tonkern> k = kerne().erzeuge(kKernName, w);
    REQUIRE(k);
    const StabSpiel s = spielAus(w);
    CHECK(ungemessen(kernTyp().tafel, w));
    // eine Note: wie rechneStabTon mit dem Daempfer am Ende der Note
    const Tonauftrag auftrag = einzelnote(69, 440.0, -6, 24000, 48000, 48000.0);
    std::vector<float> ueber, direkt;
    k->rechne(auftrag, ueber);
    StabAnschlag a = anschlag(69, -6);
    a.daempferAb = 24000;
    rechneStabTon(s, a, direkt, 48000);
    CHECK(ueber == direkt);
    // gestrichen: die Spur `bowed` ab bowedFrom (63); darunter angeschlagen
    Tonauftrag bogen = einzelnote(72, 440.0 * std::exp2(3.0 / 12.0), 0, 48000, 48000, 48000.0);
    bogen.spuren.push_back(Spur{std::string(kSpurGestrichen), {{0, 1.0}}});
    bogen.spuren.push_back(Spur{std::string(spur::kExpression), {{0, 0.5}}});
    std::vector<float> g;
    k->rechne(bogen, g);
    std::vector<float> gd;
    rechneStabTon(s, gestrichen(72, 0.5), gd, 48000);
    CHECK(g == gd);
    Tonauftrag tief = bogen;
    tief.noten.front().note = 60;
    tief.noten.front().hz = 440.0 * std::exp2((60 - 69) / 12.0);
    std::vector<float> gt, at;
    k->rechne(tief, gt);
    StabAnschlag ta = anschlag(60, 0);
    ta.daempferAb = 48000;
    rechneStabTon(s, ta, at, 48000);
    CHECK(gt == at);
    // der Motor ueber die Spur
    Tonauftrag mitMotor = auftrag;
    mitMotor.spuren.push_back(Spur{std::string(kSpurMotor), {{0, 4.0}}});
    std::vector<float> mm, md;
    k->rechne(mitMotor, mm);
    a.motor = {{0.0, 4.0}};
    rechneStabTon(s, a, md, 48000);
    CHECK(mm == md);
    // Nachklang nach dem Ende der Note: unter der Daempferleiste hoechstens ihre Abklingzeit (0,3 s)
    const Frames n = k->nachklang(auftrag, 48000 * 10);
    CHECK(n >= 1);
    CHECK(n <= static_cast<Frames>(std::ceil(0.3 * 48000.0)) + 1);
    // die Kalimba ohne Daempfer klingt laenger nach
    const std::shared_ptr<const Tonkern> kal = kerne().erzeuge(kKernName, fertigeWerte("kalimba"));
    REQUIRE(kal);
    CHECK(kal->nachklang(einzelnote(62, 293.66, 0, 4800, 48000, 48000.0), 48000 * 10) > 48000);
}

TEST_CASE("InstrumentModels 34g Rechenzeit: der schwerste Ton je Instrument und das Verhaeltnis zum Fluegel (meldet nur Zahlen)")
{
    // IK2-6 a: zwei Zahlen ohne Urteil — der Anteil an der Echtzeit und das Verhaeltnis zum einen Fluegel-Kern auf
    // derselben Maschine (Kennzahl); die Grenze von 4,6 % prueft Claude von Hand (RZ1-1 a: dreimal allein nach der
    // Taktprobe, Release-clang). Wie IC `test_stab.cpp`: der tiefste Ton, Stufe 0, 2 s, das Vibraphon mit dem Motor;
    // dazu gestrichen an `bowedFrom`. Je Ton das Beste aus drei Laeufen.
    const auto zeit = [](const std::function<void()>& f) {
        double best = 1e300;
        for (int i = 0; i < 3; ++i)
        {
            const auto a = std::chrono::steady_clock::now();
            f();
            best = std::min(best, std::chrono::duration<double>(std::chrono::steady_clock::now() - a).count());
        }
        return best;
    };
    constexpr double kDauerS = 2.0;
    const auto frames = static_cast<std::int64_t>(kDauerS * 48000);
    // der Fluegel-Kern (ohne Boden) wie der Bezug der Grenze (`*31b-3 Rechenzeit*`, RZ1-1 a): Taste 21, Stufe 0, 4 s
    // gehalten, 4,8 s Ton — kuerzer schlaegt seine Vorbereitung je Ton durch (gemessen S178: 2 s gaben 4,2 … 5,8 %)
    const std::shared_ptr<const Tonkern> fluegel = kerne().erzeuge("hammerString", vorgaben(kerne().typ("hammerString")->tafel));
    REQUIRE(fluegel);
    std::vector<float> f;
    constexpr double kFluegelS = 4.8;
    const double tFluegelGanz = zeit([&] { fluegel->rechne(einzelnote(21, 27.5, 0, 4 * 48000, static_cast<Frames>(kFluegelS * 48000), 48000.0), f); });
    const double tFluegel = tFluegelGanz / kFluegelS * kDauerS;  // auf 2 s Ton gerechnet: das Verhaeltnis der Anteile
    MESSAGE("Fluegel-Kern A0, 4 s gehalten, 4,8 s Ton: " << 100.0 * tFluegelGanz / kFluegelS << " % der Echtzeit");
    for (const Vorlage& v : vorlagen())
    {
        const StabSpiel s = spielAus(fertigeWerte(v.name));
        StabAnschlag n = anschlag(s.noteMin, 0);
        if (s.motor) n.motor = {{0.0, 3.0}};
        std::vector<float> aus;
        const double t = zeit([&] { rechneStabTon(s, n, aus, frames); });
        MESSAGE(v.name << " tiefster Ton " << s.noteMin << ": " << 100.0 * t / kDauerS << " % der Echtzeit, " << t / tFluegel
                       << " × Fluegel");
        if (s.bogen)
        {
            StabAnschlag g = gestrichen(s.bogenAb, 1.0);
            g.haltenS = kDauerS;
            const double tg = zeit([&] { rechneStabTon(s, g, aus, frames); });
            MESSAGE(v.name << " gestrichen an " << s.bogenAb << " (ff): " << 100.0 * tg / kDauerS << " % der Echtzeit, " << tg / tFluegel
                           << " × Fluegel");
        }
    }
    CHECK(tFluegel > 0.0);
}

TEST_CASE("InstrumentModels 34g Kern bar: gestimmte Werte veralten in Reihenfolge — die Masse macht Pegel und Fenster alt")
{
    Werte w = fertigeWerte("vibraphone");
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, w).empty());
    CHECK_FALSE(lumi::messung::bezugVeraltet(kerne(), kKernName, w));
    // eine Groesse des Klangs: alle gestimmten Werte veralten
    Kurve masse = w.kurve("mass");
    masse.punkte.front().wert *= 1.01;
    Werte geaendert = w;
    geaendert.setze("mass", masse);
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, geaendert).size() == 7);
    // nur der Pegel des Partners von Hand: die Fenster veralten (sie haengen an ihm), er selbst nicht
    Werte pegel = w;
    Kurve p = pegel.kurve("partnerGain");
    p.punkte.front().wert *= 1.01;
    pegel.setze("partnerGain", p);
    const std::vector<std::string> alt = lumi::messung::veralteteWerte(kerne(), kKernName, pegel);
    CHECK(alt.size() == 6);
    CHECK(std::find(alt.begin(), alt.end(), "partnerGain") == alt.end());
    // der Bezug haengt nicht am Abdruck, wohl am Ton
    Werte bezug = w;
    bezug.setze(std::string(kBezug), w.zahl(std::string(kBezug)) * 1.01);
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, bezug).empty());
    CHECK(lumi::messung::bezugVeraltet(kerne(), kKernName, bezug));
}
