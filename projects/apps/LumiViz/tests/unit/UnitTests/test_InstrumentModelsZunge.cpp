/**
 ****************************************************************************************
 * @file   test_InstrumentModelsZunge.cpp
 * @brief  InstrumentModels, Stufe 34j: Klasse 10 (durchschlagende Zunge) — der Kern `freeReed`
 *         (docs/audio/Instrumente_IC_Konzept.md 34j-2, 34j-6, ZV1 a, ZV4 a–ZV7 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine.
 *
 * - Tafel, Fabrik, Vorlagen, Werte hin und zurueck; die Kurven je Taste lesen an jeder Taste ihr Gesetz (34j-6).
 * - Die Abdruecke aus `werte/zunge_ic.txt` Bit fuer Bit, mit der Stimmung und dem Bezug aus derselben Datei, ueber den
 *   direkten Aufruf und ueber den Kern. **Seit Rechenfassung 2 (S191) rechnet der Kern selbst die Datei** — neu schreiben
 *   mit `LUMIVIZ_ZUNGE_BEZUG_SCHREIBEN=1` (stimmt alle vier, einige Sekunden); bis S190 trug sie den Beleg der IC-Vorlage
 *   (34j-1, Journal J66, J70).
 * - Der Bezug: `kern::normiereNachTafel` gleich den Zeilen `bezugLv`.
 * - Die Faelle aus IC `test_zunge` (`4409088`), angepasst an Rechenfassung 2 (Journal J70).
 * - Der Waechter „jede Taste deckt den Windbereich“ (34j-6): alle vier Instrumente, jede Taste bei `windMin`, in der
 *   Mitte und bei `windMax`, die Messkette des Pruefstands S190.
 * - Der Stimmer: die Melodica Bit fuer Bit gleich der Datei, Waechter fuer veraltete Werte; alle vier nur mit
 *   `LUMIVIZ_34J_STIMMEN=1`.
 * - Messfall `LUMIVIZ_34J_MESSUNG=<datei.md>`: je Instrument und Taste die Zielgroessen von 34j-5 wie der Pruefstand
 *   (`docs/modellierung/zunge/proben/2026-11-11_S190_pruefstand_ergebnisse.txt`), dazu die Lautheit des ganzen Tons
 *   ueber den Umfang (Befund S188) und der Bend ueber das Ventil; ohne Variable nur eine Meldung.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Pool.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentMeasure/ZungeStimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/zunge/ZungeKern.hpp"

#include "AudioCore/Lautheit.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::kern;
using namespace lumi::kern::zunge;

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr Frames kFrames = 28800;  // wie der Beleg: 0,6 s bei 48 kHz

double hzVon(int n) { return 440.0 * std::exp2((n - 69) / 12.0); }

std::string hexVon(double x)
{
    std::uint64_t b = 0;
    std::memcpy(&b, &x, sizeof b);
    char t[32];
    std::snprintf(t, sizeof t, "%016llx", static_cast<unsigned long long>(b));
    return t;
}

double ausHex(const std::string& h)
{
    const std::uint64_t b = std::stoull(h, nullptr, 16);
    double x = 0.0;
    std::memcpy(&x, &b, sizeof x);
    return x;
}

bool variable(const char* name)
{
    const char* e = std::getenv(name);
    return e != nullptr && std::string(e) == "1";
}

std::filesystem::path bezugsDatei()
{
    return std::filesystem::path(__FILE__).parent_path() / "werte" / "zunge_ic.txt";
}

/// Die Bezugsdatei (`werte/zunge_ic.txt`).
struct Beleg
{
    std::map<std::string, std::string> abdruck;
    std::map<std::string, std::string> bezugLv;
    /// je Name des Instruments und gestimmtem Wert die Punkte
    std::map<std::string, std::map<std::string, std::vector<NotenWert>>> kurven;
};

Beleg liesBeleg()
{
    Beleg r;
    std::ifstream f(bezugsDatei());
    std::string zeile;
    while (std::getline(f, zeile))
    {
        std::istringstream z(zeile);
        std::string art, name;
        z >> art >> name;
        if (art == "abdruck" || art == "bezugLv")
        {
            std::string wert;
            z >> wert;
            (art == "abdruck" ? r.abdruck : r.bezugLv)[name] = wert;
        }
        else if (art == "kurve")
        {
            std::string welche, hex;
            int note = 0;
            z >> welche >> note >> hex;
            r.kurven[name][welche].push_back(NotenWert{note, ausHex(hex)});
        }
    }
    return r;
}

const Beleg& beleg()
{
    static const Beleg b = liesBeleg();
    return b;
}

const Vorlage& vorlage(std::string_view name)
{
    for (const Vorlage& v : vorlagen())
        if (v.name == name) return v;
    FAIL("keine Vorlage " << name);
    return vorlagen().front();
}

Kurve kurveAus(const std::vector<NotenWert>& punkte)
{
    Kurve c;
    c.punkte = punkte;
    c.belegkraft = Belegkraft::Rechnung;
    c.herkunft = "werte/zunge_ic.txt";
    return c;
}

/// Die Werte einer Vorlage mit der Stimmung aus der Datei; mit `mitBezug` der Bezug aus der Datei, sonst 1.
Werte gestimmteWerte(const Vorlage& v, bool mitBezug)
{
    Werte w = werteAus(v.spiel);
    for (const std::string& n : gestimmteNamen()) w.setze(n, kurveAus(beleg().kurven.at(v.spiel.name).at(n)));
    w.setze(std::string(kBezug), mitBezug ? ausHex(beleg().bezugLv.at(v.spiel.name)) : 1.0);
    const std::vector<std::string> f = richteWerte(kernTyp().tafel, w);
    REQUIRE_MESSAGE(f.empty(), v.name << ": " << (f.empty() ? std::string() : f.front()));
    return w;
}

/// Das Spiel gestimmt, mit Bezug — ueber die Tafel, wie die App es baut.
ZungenSpiel spielGestimmt(std::string_view name)
{
    ZungenSpiel s = spielAus(gestimmteWerte(vorlage(name), true));
    s.name = vorlage(name).spiel.name;
    return s;
}

const ZungenSpiel& akkordeon()
{
    static const ZungenSpiel a = spielGestimmt("accordion");
    return a;
}

ZungenAnschlag ton(int note, double e, double haltenS = 0.8)
{
    ZungenAnschlag a;
    a.note = note;
    a.expression = e;
    a.haltenS = haltenS;
    return a;
}

std::string abdruckVon(const ZungenSpiel& s, const ZungenAnschlag& a, std::int64_t frames = kFrames)
{
    std::vector<float> t;
    rechneZungenTon(s, a, t, frames);
    return abdruck(t);
}

/// Die Faelle der Abdruecke: Name und Anschlag, je Spiel (Namen wie der IC-Beleg; `_bend` mit −20 Cent).
std::vector<std::pair<std::string, ZungenAnschlag>> abdruckFaelle(const ZungenSpiel& s)
{
    std::vector<std::pair<std::string, ZungenAnschlag>> f;
    const std::string basis = "zunge_" + s.name + "_m";
    const int tief = std::max(s.noteMin, s.bassAb);
    const int mitte = (tief + s.noteMax) / 2;
    const struct
    {
        const char* name;
        double e;
    } stufen[] = {{"e0", 0.0}, {"e05", 0.5}, {"e1", 1.0}};
    for (const int n : {tief, mitte, s.noteMax})
        for (const auto& st : stufen) f.emplace_back(basis + std::to_string(n) + "_" + st.name, ton(n, st.e, 0.4));
    ZungenAnschlag vib = ton(mitte, 0.5, 0.4);
    vib.vibratoCent = 20.0;
    vib.vibratoHz = 5.5;
    f.emplace_back(basis + std::to_string(mitte) + "_vibrato", vib);
    ZungenAnschlag bend = ton(mitte, 0.5, 0.4);
    bend.pitchCent = -20.0;
    f.emplace_back(basis + std::to_string(mitte) + "_bend", bend);
    if (s.bassAb > 0)
    {
        for (const int n : {29, 40, 52}) f.emplace_back("zunge_" + s.name + "_bass_m" + std::to_string(n) + "_e05", ton(n, 0.5, 0.4));
        ZungenAnschlag w = ton(69, 0.5, 0.4);
        w.balgwechselS = 0.2;
        f.emplace_back("zunge_" + s.name + "_m69_balgwechsel", w);
        f.emplace_back("zunge_" + s.name + "_m69_mf", ton(69, 0.5, 0.8));
    }
    return f;
}

/// Ein Ton ueber den Kern (Rechenweg Note, beginn 0) mit den Werten aus der Datei.
std::string kernAbdruck(std::string_view name, int note, Frames halten, std::vector<Spur> spuren)
{
    std::vector<std::string> fehler;
    const std::shared_ptr<const Tonkern> k = kerne().erzeuge(kKernName, gestimmteWerte(vorlage(name), true), &fehler);
    REQUIRE_MESSAGE(k != nullptr, (fehler.empty() ? std::string() : fehler.front()));
    Tonauftrag a = einzelnote(note, hzVon(note), 0, halten, kFrames, 48000.0);
    a.spuren = std::move(spuren);
    std::vector<float> t;
    k->rechne(a, t);
    return abdruck(t);
}

Spur spurAm(std::string_view name, double wert) { return Spur{std::string(name), {SpurPunkt{0, wert}}}; }

double rms(const std::vector<float>& t)
{
    double s = 0.0;
    for (const float x : t) s += double(x) * double(x);
    return t.empty() ? 0.0 : std::sqrt(s / static_cast<double>(t.size()));
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < t.size(); ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * double(t[ab + i]) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

double centVon(double hz, int note) { return hz > 0.0 ? 1200.0 * std::log2(hz / hzVon(note)) : 0.0; }

/// Die gestimmten Kurven einer Stimmung mit ihren Namen, in der Reihenfolge der Tafel.
std::vector<std::pair<std::string, Kurve>> gestimmteKurven(const lumi::messung::ZungenStimmung& st)
{
    std::vector<std::pair<std::string, Kurve>> v = {{std::string(kEigenfrequenz), st.eigenfrequenz}};
    for (int k = 0; k < kBendStufen; ++k) v.emplace_back(bendStufenName(k), st.bendCentJeStufe[static_cast<std::size_t>(k)]);
    v.emplace_back(std::string(kBendGrenzeOeffnung), st.bendGrenzeOeffnung);
    v.emplace_back(std::string(kBendGrenzeCent), st.bendGrenzeCent);
    return v;
}

}  // namespace

TEST_CASE("InstrumentModels 34j-6 Kern freeReed: die Bezugsdatei neu schreiben (LUMIVIZ_ZUNGE_BEZUG_SCHREIBEN=1)")
{
    if (!variable("LUMIVIZ_ZUNGE_BEZUG_SCHREIBEN"))
    {
        MESSAGE("LUMIVIZ_ZUNGE_BEZUG_SCHREIBEN=1 stimmt alle vier Instrumente und schreibt werte/zunge_ic.txt neu");
        return;
    }
    std::ostringstream o;
    o << "# Bezug des Kerns freeReed, Rechenfassung 2 (34j-6, S191) — gerechnet vom Kern in LumiViz (test_InstrumentModelsZunge,\n"
         "# LUMIVIZ_ZUNGE_BEZUG_SCHREIBEN=1, Release-clang; Testing-MSVC gibt dieselben Zeilen) — nicht von Hand aendern\n"
         "# Vermerk: bis S190 trug diese Datei den Beleg der IC-Vorlage (34j-1, Journal J66; Stand in Git bis 4d6ada93); mit\n"
         "#   Rechenfassung 2 (Groessen je Taste, ein Wind, f0 am mittleren Druck) sind alle Zeilen bewusst neu (Journal J70)\n"
         "# abdruck <name> <sha256>: 28800 Frames bei 48 kHz; _e0/_e05/_e1 = Expression; 0,4 s gehalten, ausser _m69_mf (0,8 s);\n"
         "#   _bend = pitch -20 Cent (ueber die gestimmte Kurve der Taste); der Bezug aus der Zeile bezugLv\n"
         "# bezugLv <name> <hex der 64 Bit> <Zahl>: ausgangBezug nach kern::normiereNachTafel (Mitte noteMin…noteMax, 1 s)\n"
         "# kurve <name> <gestimmter Wert> <note> <hex> <Zahl>: die Stimmung (zungenStimmung, am mittleren Druck) — reedFrequency,\n"
         "#   bendStep1 … bendStep7, bendLimitOpening, bendLimitCent\n";
    std::vector<std::string> kurven, bezuege, abdruecke;
    for (const Vorlage& v : vorlagen())
    {
        const auto beginn = std::chrono::steady_clock::now();
        const lumi::messung::ZungenStimmung st = lumi::messung::zungenStimmung(v.spiel);
        CHECK_MESSAGE(st.stumm.empty(), v.name << ": spricht am mittleren Druck nicht an " << st.stumm.size() << " Tasten");
        Werte w = werteAus(v.spiel);
        for (const auto& [n, k] : gestimmteKurven(st)) w.setze(n, k);
        REQUIRE(richteWerte(kernTyp().tafel, w).empty());
        std::vector<std::string> fehler;
        const double bezug = normiereNachTafel(kerne(), kKernName, w, &fehler);
        REQUIRE_MESSAGE(fehler.empty(), (fehler.empty() ? std::string() : fehler.front()));
        w.setze(std::string(kBezug), bezug);
        ZungenSpiel s = spielAus(w);
        s.name = v.spiel.name;
        for (const auto& [n, k] : gestimmteKurven(st))
            for (const NotenWert& p : k.punkte)
            {
                char z[200];
                std::snprintf(z, sizeof z, "kurve %s %s %d %s %.17g", s.name.c_str(), n.c_str(), p.note, hexVon(p.wert).c_str(), p.wert);
                kurven.emplace_back(z);
            }
        char b[160];
        std::snprintf(b, sizeof b, "bezugLv %s %s %.17g", s.name.c_str(), hexVon(bezug).c_str(), bezug);
        bezuege.emplace_back(b);
        for (const auto& [name, a] : abdruckFaelle(s)) abdruecke.push_back("abdruck " + name + " " + abdruckVon(s, a));
        MESSAGE(v.name << ": gestimmt in " << std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count()
                       << " s, Bezug " << bezug);
    }
    for (const auto* liste : {&abdruecke, &bezuege, &kurven})
        for (const std::string& z : *liste) o << z << "\n";
    std::ofstream f(bezugsDatei(), std::ios::binary);
    f << o.str();
    REQUIRE(f.good());
    MESSAGE("geschrieben: " << bezugsDatei().string() << " (" << abdruecke.size() << " Abdruecke, " << kurven.size() << " Kurvenpunkte)");
}

TEST_CASE("InstrumentModels 34j-2 Kern freeReed: Tafel, Fabrik, Vorlagen, Werte hin und zurueck")
{
    const KernTyp& t = kernTyp();
    const std::vector<std::string> fehler = pruefeTafel(t.tafel);
    CHECK_MESSAGE(fehler.empty(), (fehler.empty() ? std::string() : fehler.front()));
    REQUIRE(kerne().typ(kKernName) != nullptr);
    CHECK(t.tafel.rechenFassung == 2);
    CHECK(t.tafel.bezugWert == kBezug);
    CHECK(t.tafel.umfangVon == "noteMin");
    CHECK(t.tafel.umfangBis == "noteMax");
    CHECK(t.tafel.gruppen == std::vector<std::string>{"Keys", "Reed", "Chamber", "Wind", "Ranks", "Bass ranks", "Case", "Tunable", "Output"});
    CHECK(t.auskunft.rechenweg == Rechenweg::Note);
    CHECK_FALSE(t.auskunft.koerperAnDerSchicht);
    CHECK(t.auskunft.spuren == std::vector<std::string>{"expression", "pitch", "vibrato", "bellows"});
    CHECK(t.auskunft.anregungDauernd);
    CHECK(t.auskunft.innere == std::vector<std::string>{std::string(kInnereSpitze)});
    std::vector<std::string> gestimmt, kurven;
    for (const ParameterAngabe& a : t.tafel.werte)
    {
        CAPTURE(a.name);
        CHECK_FALSE(a.quellen.empty());
        CHECK_FALSE(a.herkunft.empty());
        if (a.gestimmt) gestimmt.push_back(a.name);
        else if (a.art == WertArt::Kurve) kurven.push_back(a.name);
    }
    CHECK(gestimmt == gestimmteNamen());
    CHECK(gestimmt == std::vector<std::string>{"reedFrequency", "bendStep1", "bendStep2", "bendStep3", "bendStep4", "bendStep5", "bendStep6",
                                               "bendStep7", "bendLimitOpening", "bendLimitCent"});
    CHECK(kurven == std::vector<std::string>{"reedLength", "reedWidth", "reedMassPerArea", "reedDamping", "reedRest", "sideGap", "chamberVolume",
                                             "channelLength", "channelArea", "valveArea"});

    std::vector<std::string> namen;
    for (const Vorlage& v : vorlagen()) namen.push_back(v.name);
    CHECK(namen == std::vector<std::string>{"accordion", "harmonium", "melodica", "harmonica"});
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        // ungestimmt fehlen die gestimmten Werte (IK2-11 a: ein Fehler, kein stiller Ersatz)
        Werte roh = werteAus(v.spiel);
        CHECK(richteWerte(t.tafel, roh).size() == gestimmteNamen().size());
        // jede Kurve je Taste traegt Stuetzpunkte im Umfang, Herkunft und Quellen
        for (const ParameterAngabe& a : t.tafel.werte)
            if (a.art == WertArt::Kurve && !a.gestimmt)
            {
                CAPTURE(a.name);
                const Kurve& k = roh.kurve(a.name);
                REQUIRE_FALSE(k.punkte.empty());
                CHECK(k.punkte.front().note == v.spiel.noteMin);
                CHECK(k.punkte.back().note == v.spiel.noteMax);
                CHECK_FALSE(k.herkunft.empty());
                CHECK_FALSE(k.quellen.empty());
            }
        // gestimmt: die Vorgaben liegen in den Grenzen, Richten aendert nichts, zurueck ins Spiel und wieder in Werte gleich
        Werte w = werteAus(v.spiel);
        const Werte g = gestimmteWerte(v, true);
        for (const std::string& n : gestimmteNamen()) w.setze(n, *g.finde(n));
        w.setze(std::string(kBezug), *g.finde(kBezug));
        const Werte vorher = w;
        const std::vector<std::string> f = richteWerte(t.tafel, w);
        CHECK_MESSAGE(f.empty(), (f.empty() ? std::string() : f.front()));
        CHECK(w == vorher);
        CHECK(werteAus(spielAus(w)) == w);
    }
    // die Choere als Listen: das Akkordeon Musette und die Bass-Seite 8′ + 4′ (ZV6 a)
    const ZungenSpiel& a = akkordeon();
    REQUIRE(a.choere.size() == 2);
    CHECK(a.choere[1].schwebungA4Hz == 2.0);
    REQUIRE(a.bassChoere.size() == 2);
    CHECK(a.bassChoere[0].halbtoene == 0);
    CHECK(a.bassChoere[1].halbtoene == 12);
    CHECK(a.bassChoere[1].pegel == 0.7);
    CHECK(a.bassAb == 53);
    CHECK(a.windMinPa == 200.0);
    CHECK(a.windMaxPa == 1600.0);
}

TEST_CASE("InstrumentModels 34j-6 Kern freeReed: die Kurven je Taste lesen an jeder Taste ihr Gesetz")
{
    // S190 34j-5 §3, §4: Wert an C4 mal (f/f_C4)^Exponent; die Kurve trifft das Gesetz an jeder Taste, auch am Knick F3
    const ZungenSpiel s = vorlage("accordion").spiel;
    const auto gesetz = [](double c4, double hoch, int n) { return c4 * std::pow(hzVon(n) / hzVon(60), hoch); };
    double groessteAbweichung = 0.0;
    for (int n = s.noteMin; n <= s.noteMax; ++n)
    {
        CAPTURE(n);
        const ZungeAnDerTaste z = zungeAn(s, n);
        const bool bass = n < 53;
        const double soll[] = {gesetz(32.8e-3, -0.42, n), gesetz(3.45e-3, -0.2, n), bass ? gesetz(0.288e-3, -0.8, n) : gesetz(0.3e-3, -0.7, n),
                               bass ? gesetz(0.14e-3, -0.4, n) : gesetz(0.14e-3, -0.7, n), gesetz(60e-3, -0.42, n), gesetz(6e-6, -0.9, n),
                               gesetz(1e-6, -0.9, n)};
        const double ist[] = {z.laenge, z.breite, z.ruhe, z.spalt, z.kanalLaenge, z.ventil, z.kanzelle};
        for (std::size_t i = 0; i < std::size(soll); ++i) groessteAbweichung = std::max(groessteAbweichung, std::abs(ist[i] / soll[i] - 1.0));
        const double mu = (bass ? gesetz(2.388, -1.2, n) : gesetz(2.92, -0.7, n));
        groessteAbweichung = std::max(groessteAbweichung, std::abs(z.masse / (mu * 0.5 * z.laenge * z.breite) - 1.0));
        CHECK(z.zeta == doctest::Approx(0.003).epsilon(1e-12));
    }
    MESSAGE("Akkordeon: groesste Abweichung der Kurven vom Gesetz " << groessteAbweichung);
    CHECK(groessteAbweichung < 1e-12);
    // die Masse an C4 ist die der Zunge von Adachi et al. 2026 Tab. 1: (2/3) · 0,248 g
    CHECK(zungeAn(s, 60).masse == doctest::Approx(2.0 / 3.0 * 0.248e-3).epsilon(1e-3));
    // der Wind: geometrisch ueber die Expression, die Mitte am mittleren Druck
    CHECK(windBei(s, 0.0) == 200.0);
    CHECK(windBei(s, 1.0) == doctest::Approx(1600.0).epsilon(1e-12));
    CHECK(windMitte(s) == doctest::Approx(std::sqrt(200.0 * 1600.0)).epsilon(1e-12));
}

TEST_CASE("InstrumentModels 34j-6 Kern freeReed: die Abdruecke aus werte/zunge_ic.txt Bit fuer Bit")
{
    const Beleg& b = beleg();
    REQUIRE(b.abdruck.size() == 49);
    int geprueft = 0;
    for (const Vorlage& v : vorlagen())
    {
        const ZungenSpiel s = spielGestimmt(v.name);
        for (const auto& [name, a] : abdruckFaelle(s))
        {
            CAPTURE(name);
            REQUIRE(b.abdruck.count(name) == 1);
            CHECK(abdruckVon(s, a) == b.abdruck.at(name));
            ++geprueft;
        }
    }
    CHECK(geprueft == 49);

    // ueber den Kern: Expression und Bass-Seite als Spuren am Beginn (Stufe 0)
    const auto ueberKern = [&](const Vorlage& v, int n, double e, Frames halten, std::vector<Spur> mehr = {}) {
        std::vector<Spur> spuren = {spurAm(spur::kExpression, e)};
        for (Spur& x : mehr) spuren.push_back(std::move(x));
        return kernAbdruck(v.name, n, halten, std::move(spuren));
    };
    CHECK(ueberKern(vorlage("accordion"), 69, 0.5, 38400) == b.abdruck.at("zunge_Akkordeon_m69_mf"));
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const int tief = std::max(v.spiel.noteMin, v.spiel.bassAb);
        const int mitte = (tief + v.spiel.noteMax) / 2;
        const std::string basis = "zunge_" + v.spiel.name + "_m";
        for (const int n : {tief, mitte, v.spiel.noteMax})
            for (const auto& [st, e] : {std::pair{"e0", 0.0}, std::pair{"e05", 0.5}, std::pair{"e1", 1.0}})
                CHECK(ueberKern(v, n, e, 19200) == b.abdruck.at(basis + std::to_string(n) + "_" + st));
        // ZV9 a: `pitch` am Beginn ueber die gestimmte Kurve; ohne Bend (Harmonium, Mundharmonika) klingt er wie ohne Spur
        const std::string bend = basis + std::to_string(mitte) + "_bend", ohne = basis + std::to_string(mitte) + "_e05";
        CHECK(ueberKern(v, mitte, 0.5, 19200, {spurAm(spur::kPitch, -20.0)}) == b.abdruck.at(bend));
        CHECK((b.abdruck.at(bend) == b.abdruck.at(ohne)) == !v.spiel.bendUeberVentil);
    }
    CHECK(ueberKern(vorlage("accordion"), 40, 0.5, 19200) == b.abdruck.at("zunge_Akkordeon_bass_m40_e05"));
    // ohne Spur `expression` die volle Staerke (SK4 a)
    CHECK(kernAbdruck("melodica", 68, 19200, {}) == b.abdruck.at("zunge_Melodica_m68_e1"));
    // das Vibrato des Kerns: 5 Hz (IC-Vorgabe), sonst wie der direkte Aufruf
    {
        ZungenAnschlag vib = ton(68, 0.5, 0.4);
        vib.vibratoCent = 20.0;
        CHECK(ueberKern(vorlage("melodica"), 68, 0.5, 19200, {spurAm(spur::kVibrato, 20.0)}) == abdruckVon(spielGestimmt("melodica"), vib));
    }
}

TEST_CASE("InstrumentModels 34j-2 Kern freeReed: der Bezug nach der Tafel")
{
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        std::vector<std::string> fehler;
        const double lv = normiereNachTafel(kerne(), kKernName, gestimmteWerte(v, false), &fehler);
        REQUIRE_MESSAGE(fehler.empty(), (fehler.empty() ? std::string() : fehler.front()));
        CHECK(hexVon(lv) == beleg().bezugLv.at(v.spiel.name));
        MESSAGE(v.name << ": Bezug " << lv);
    }
}

TEST_CASE("InstrumentModels 34j-2 IC test_zunge: spricht selbst an und schwingt etwas unter f0, klingt nach dem Wind aus")
{
    const ZungenSpiel& s = akkordeon();
    const double f0 = wertAn(s.eigenfrequenzJeNote, 60, Skala::Logarithmisch);
    std::vector<double> t;
    const lumi::messung::ZungenTon m = lumi::messung::messeEineZunge(s, f0, 60, ton(60, 0.5), 48000, &t);
    REQUIRE(m.schwingt);
    MESSAGE("Zunge C4: f0 " << f0 << " Hz, spielt " << m.spielHz << " Hz, Amplitude " << 100.0 * m.amplitudeRel << " % (Cottingham: 15 %)");
    CHECK(m.spielHz < f0);
    CHECK(m.spielHz > 0.97 * f0);
    CHECK(m.amplitudeRel > 0.02);
    double halt = 0.0, nach = 0.0;
    for (std::size_t i = 28800; i < 33600; ++i) halt += t[i] * t[i];
    for (std::size_t i = 43200; i < 48000; ++i) nach += t[i] * t[i];
    CHECK(nach < 0.01 * halt);
}

TEST_CASE("InstrumentModels 34j-6 IC test_zunge: Akkordeon jede dritte Diskantnote in der Mitte auf ±10 Cent, ff schneller da")
{
    // ZV5 a: ±10 Cent am mittleren Druck; die Tonhoehe folgt dem Druck (bei pp hoeher, bei ff tiefer — Berichtsgroesse)
    const ZungenSpiel& s = akkordeon();
    int getroffen = 0, gesamt = 0;
    double ppHoch = -1e9, ffTief = 1e9;
    for (int note = s.bassAb; note <= s.noteMax; note += 3)
    {
        CAPTURE(note);
        const lumi::messung::ZungenTon pp = lumi::messung::messeErsteZunge(s, ton(note, 0.0), 38400);
        const lumi::messung::ZungenTon mf = lumi::messung::messeErsteZunge(s, ton(note, 0.5), 38400);
        const lumi::messung::ZungenTon ff = lumi::messung::messeErsteZunge(s, ton(note, 1.0), 38400);
        REQUIRE(pp.schwingt);
        REQUIRE(mf.schwingt);
        REQUIRE(ff.schwingt);
        ++gesamt;
        if (std::abs(centVon(mf.spielHz, note)) < 10.0) ++getroffen;
        ppHoch = std::max(ppHoch, centVon(pp.spielHz, note));
        ffTief = std::min(ffTief, centVon(ff.spielHz, note));
        CHECK(ff.einschwingS <= pp.einschwingS + 0.02);
    }
    MESSAGE("Akkordeon (YIN, M-Chor): " << getroffen << " von " << gesamt << " in der Mitte auf ±10 Cent; pp bis " << ppHoch << ", ff bis " << ffTief
                                        << " Cent");
    CHECK(getroffen == gesamt);
}

TEST_CASE("InstrumentModels 34j-2 IC test_zunge: Akkordeon Musette schwebt mit 2 Hz an A4 (Hergert)")
{
    std::vector<float> t;
    rechneZungenTon(akkordeon(), ton(69, 0.5, 3.0), t, 144000);
    std::vector<double> h;
    for (std::size_t i = 24000; i + 2400 <= 120000; i += 2400) h.push_back(band(t, i, 2400, 440.0));
    double best = 0.0, bestHz = 0.0;
    for (double fm = 0.5; fm <= 6.0; fm += 0.05)
    {
        std::complex<double> s(0.0, 0.0);
        double mittel = 0.0;
        for (const double x : h) mittel += x / h.size();
        for (std::size_t i = 0; i < h.size(); ++i) s += (h[i] - mittel) * std::exp(std::complex<double>(0.0, -2.0 * kPi * fm * i * 0.05));
        if (std::abs(s) > best)
        {
            best = std::abs(s);
            bestHz = fm;
        }
    }
    MESSAGE("Akkordeon A4 Musette: Schwebung " << bestHz << " Hz");
    CHECK(bestHz == doctest::Approx(2.0).epsilon(0.15));
}

TEST_CASE("InstrumentModels 34j-6 IC test_zunge: das Ventil senkt die Tonhoehe, Bass-Seite 8′ und 4′, Balgwechsel")
{
    const ZungenSpiel& s = akkordeon();
    std::vector<float> t0;
    const lumi::messung::ZungenTon m0 = lumi::messung::messeErsteZunge(s, ton(64, 0.6), 38400);
    rechneZungenTon(s, ton(64, 0.6), t0, 38400);
    ZungenAnschlag b = ton(64, 0.6);
    b.ventilFaktor = 0.5;
    const lumi::messung::ZungenTon mb = lumi::messung::messeErsteZunge(s, b, 38400);
    MESSAGE("Akkordeon E4: Ventil halb ergibt " << 1200.0 * std::log2(mb.spielHz / m0.spielHz) << " Cent");
    REQUIRE(mb.schwingt);
    CHECK(mb.spielHz < m0.spielHz * std::exp2(-2.0 / 1200.0));
    // die Bass-Seite: beide Fusslagen klingen (ZV6 a: 8′ auf der Note, 4′ eine Oktave darueber)
    for (const int n : {29, 40, 52})
    {
        CAPTURE(n);
        std::vector<float> bass;
        rechneZungenTon(s, ton(n, 0.5), bass, 38400);
        CHECK(rms(bass) > 0.0);
        CHECK(band(bass, 14400, 16384, hzVon(n)) > 0.0);
        CHECK(band(bass, 14400, 16384, hzVon(n + 12)) > 0.0);
    }
    ZungenAnschlag w = ton(64, 0.6);
    w.balgwechselS = 0.4;
    std::vector<float> tw;
    rechneZungenTon(s, w, tw, 38400);
    CHECK(tw != t0);
}

TEST_CASE("InstrumentModels 34j-2 IC test_zunge: Harmonium, Melodica, Mundharmonika spielen ihre Mitte auf ±10 Cent")
{
    for (const char* name : {"harmonium", "melodica", "harmonica"})
    {
        CAPTURE(name);
        const ZungenSpiel s = spielGestimmt(name);
        const int n = (s.noteMin + s.noteMax) / 2;
        const lumi::messung::ZungenTon m = lumi::messung::messeErsteZunge(s, ton(n, 0.5), 38400);
        REQUIRE(m.schwingt);
        CHECK(std::abs(centVon(m.spielHz, n)) < 10.0);
    }
}

TEST_CASE("InstrumentModels 34j-2 IC test_zunge: Determinismus ueber Threads, Innere Groesse reedTip")
{
    const ZungenSpiel& s = akkordeon();
    std::vector<float> a;
    rechneZungenTon(s, ton(69, 0.5), a, 28800);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneZungenTon(s, ton(69, 0.5), q[i], 28800); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
    CHECK(abdruck(a) == beleg().abdruck.at("zunge_Akkordeon_m69_mf"));
    // die innere Groesse ueber den Kern: gleich der Messung der ersten Zunge
    std::vector<std::string> fehler;
    const std::shared_ptr<const Tonkern> k = kerne().erzeuge(kKernName, gestimmteWerte(vorlage("accordion"), true), &fehler);
    REQUIRE(k != nullptr);
    Tonauftrag auftrag = einzelnote(69, hzVon(69), 0, 38400, 28800, 48000.0);
    auftrag.spuren = {spurAm(spur::kExpression, 0.5)};
    Innere innere;
    innere.gewuenscht = {std::string(kInnereSpitze)};
    std::vector<float> t;
    k->rechne(auftrag, t, &innere);
    REQUIRE(innere.ergebnis.size() == 1);
    REQUIRE(innere.ergebnis.front().werte.size() == 1);
    CHECK(innere.ergebnis.front().werte.front() == lumi::messung::messeErsteZunge(s, ton(69, 0.5), 28800).amplitudeRel);
    CHECK(innere.ergebnis.front().werte.front() > 0.02);
    // Nachklang bis 34j-7 hoechstens 1 s, ohne Note still
    CHECK(k->nachklang(auftrag, 96000) == 48000);
    CHECK(k->nachklang(auftrag, 100) == 100);
    Tonauftrag leer;
    leer.frames = 10;
    k->rechne(leer, t);
    CHECK(t == std::vector<float>(10, 0.0f));
}

TEST_CASE("InstrumentModels 34j-7 Kern freeReed: laufender Wind — konstante Spur Bit fuer Bit, Balgwechsel als Spur")
{
    // eine Spur mit gleichen Punkten rechnet wie ein fester Wert (34j-7: „konstante Spur Bit fuer Bit wie 34j-6“)
    const Beleg& b = beleg();
    const auto zwei = [](std::string_view name, double w) { return Spur{std::string(name), {SpurPunkt{0, w}, SpurPunkt{12000, w}}}; };
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const int tief = std::max(v.spiel.noteMin, v.spiel.bassAb), mitte = (tief + v.spiel.noteMax) / 2;
        const std::string basis = "zunge_" + v.spiel.name + "_m" + std::to_string(mitte);
        CHECK(kernAbdruck(v.name, mitte, 19200, {zwei(spur::kExpression, 0.5)}) == b.abdruck.at(basis + "_e05"));
        CHECK(kernAbdruck(v.name, mitte, 19200, {zwei(spur::kExpression, 0.5), zwei(spur::kPitch, -20.0)}) == b.abdruck.at(basis + "_bend"));
        CHECK(kernAbdruck(v.name, mitte, 19200, {zwei(spur::kExpression, 0.5), zwei(kSpurBalg, 0.0)}) == b.abdruck.at(basis + "_e05"));
    }
    // das Vibrato als konstante Spur gleich dem festen Wert
    CHECK(kernAbdruck("melodica", 68, 19200, {zwei(spur::kExpression, 0.5), zwei(spur::kVibrato, 20.0)}) ==
          kernAbdruck("melodica", 68, 19200, {spurAm(spur::kExpression, 0.5), spurAm(spur::kVibrato, 20.0)}));
    // der Balgwechsel als Spur (0 → 1 bei 0,2 s): der Druck bricht ein wie beim festen Wechsel `balgwechselS`, nur an
    // diesem Frame; davor gleich dem Ton ohne Wechsel
    ZungenAnschlag w = ton(69, 0.5, 0.4);
    w.balgVerlauf = Spur{std::string(kSpurBalg), {SpurPunkt{0, 0.0}, SpurPunkt{9600, 0.0}, SpurPunkt{9600, 1.0}}};
    std::vector<float> mit, ohne, fest;
    rechneZungenTon(akkordeon(), w, mit, kFrames);
    rechneZungenTon(akkordeon(), ton(69, 0.5, 0.4), ohne, kFrames);
    ZungenAnschlag f = ton(69, 0.5, 0.4);
    f.balgwechselS = 0.2;
    rechneZungenTon(akkordeon(), f, fest, kFrames);
    CHECK(std::equal(mit.begin(), mit.begin() + 9600, ohne.begin()));
    CHECK(mit != ohne);
    const auto energie = [](const std::vector<float>& t, std::size_t von, std::size_t bis) {
        double e = 0.0;
        for (std::size_t i = von; i < bis; ++i) e += double(t[i]) * double(t[i]);
        return e;
    };
    // im Einbruch leiser, und so tief wie der feste Wechsel (auf 1 %)
    CHECK(energie(mit, 10080, 12480) < 0.8 * energie(ohne, 10080, 12480));
    CHECK(energie(mit, 9600, 19200) == doctest::Approx(energie(fest, 9600, 19200)).epsilon(0.01));
}

TEST_CASE("InstrumentModels 34j-6 Waechter freeReed: jede Taste deckt den Windbereich")
{
    // 34j-6: jede Zunge spricht bei windMin, in der Mitte und bei windMax an; in der Mitte auf ±10 Cent (ZV5 a); die
    // Lautheit der Zungen ueber den Umfang hoechstens 15 LU (LP3 a) — die Messkette des Pruefstands S190.
    // Benannte Ausnahme (S190 §4, §8 „F1 bei 200 Pa“): die tiefste Basszunge F1 waechst bei windMin nach 1 s noch — sie
    // spricht, nur langsamer als die Messung wartet; geprueft wird, dass sie waechst und es die einzige ist.
    const auto ausnahme = [](std::string_view name, int note, int stufe) { return name == "accordion" && note == 29 && stufe == 0; };
    const auto beginn = std::chrono::steady_clock::now();
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const ZungenSpiel s = spielGestimmt(v.name);
        const lumi::messung::ZungenWindbereich w = lumi::messung::messeWindbereich(s);
        int spricht[3] = {0, 0, 0};
        double lo[3] = {1e9, 1e9, 1e9}, hi[3] = {-1e9, -1e9, -1e9}, centMitte = 0.0;
        for (std::size_t i = 0; i < w.noten.size(); ++i)
        {
            const lumi::messung::ZungenTaste* m[3] = {&w.min[i], &w.mitte[i], &w.max[i]};
            for (int j = 0; j < 3; ++j)
            {
                CAPTURE(w.noten[i]);
                CAPTURE(j);
                if (ausnahme(v.name, w.noten[i], j))
                {
                    CHECK(m[j]->waechst);
                    CHECK(m[j]->ampRuhe > 0.25);
                    continue;
                }
                CHECK(m[j]->spricht);
                if (!m[j]->spricht) continue;
                ++spricht[j];
                lo[j] = std::min(lo[j], m[j]->lufs);
                hi[j] = std::max(hi[j], m[j]->lufs);
            }
            centMitte = std::max(centMitte, std::abs(centVon(w.mitte[i].hz, w.noten[i])));
        }
        MESSAGE(v.name << ": spricht " << spricht[0] << " / " << spricht[1] << " / " << spricht[2] << " von " << w.noten.size()
                       << ", Lautheit ueber den Umfang " << hi[0] - lo[0] << " / " << hi[1] - lo[1] << " / " << hi[2] - lo[2]
                       << " LU, in der Mitte bis " << centMitte << " Cent");
        CHECK(centMitte < 10.0);
        for (int j = 0; j < 3; ++j) CHECK(hi[j] - lo[j] <= 15.0);
    }
    MESSAGE("Windbereich aller vier in " << std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count() << " s");
}

TEST_CASE("InstrumentModels 34j-6 Waechter freeReed: der Bend je Taste — monoton, Grenze, Umkehrung (ZV7 a–ZV9 a)")
{
    for (const char* name : {"accordion", "melodica"})
    {
        CAPTURE(name);
        const ZungenSpiel s = spielGestimmt(name);
        REQUIRE(s.bendUeberVentil);
        int nichtMonoton = 0;
        double groessterFehler = 0.0;
        for (int n = s.noteMin; n <= s.noteMax; ++n)
        {
            CAPTURE(n);
            const double grenzeF = wertAn(s.bendGrenzeOeffnung, n, Skala::Logarithmisch);
            const double grenzeC = wertAn(s.bendGrenzeCent, n, Skala::Linear);
            CHECK(grenzeC <= 0.0);
            CHECK(grenzeF <= 1.0);
            // die Kurve faellt mit der Oeffnung (eindeutig umkehrbar — Kipppunkt von ZV7 a), bis zur Grenze
            double vorher = 0.0;
            for (int k = 0; k < kBendStufen && bendStufe(k) > grenzeF; ++k)
            {
                const double c = wertAn(s.bendCentJeStufe[static_cast<std::size_t>(k)], n, Skala::Linear);
                if (c > vorher + 0.5) ++nichtMonoton;
                vorher = c;
            }
            if (grenzeC > vorher + 0.5) ++nichtMonoton;
            // die Umkehrung: −5 / −10 / −20 Cent am mittleren Druck, gemessen gegen offen, auf 2,5 Cent (ZV8 a: Ablage bis 2 Cent)
            if (n % 4 != 1) continue;
            const double f0 = wertAn(s.eigenfrequenzJeNote, n, Skala::Logarithmisch);
            const double offen = lumi::messung::messeZungenTaste(s, f0, n, windMitte(s)).hz;
            for (const double ziel : {-5.0, -10.0, -20.0})
            {
                const double f = ventilFuerCent(s, n, ziel);
                CHECK(f <= 1.0);
                CHECK(f >= grenzeF * (1.0 - 1e-12));
                const lumi::messung::ZungenTaste m = lumi::messung::messeZungenTaste(s, f0, n, windMitte(s), f);
                REQUIRE(m.spricht);
                const double ist = 1200.0 * std::log2(m.hz / offen);
                groessterFehler = std::max(groessterFehler, std::abs(ist - std::max(ziel, grenzeC)));
            }
            CHECK(ventilFuerCent(s, n, 20.0) == 1.0);  // nur abwaerts
            CHECK(ventilFuerCent(s, n, 0.0) == 1.0);
        }
        // ZV7 a: an F3 tief (S190 §7: bis −33 Cent bei 0,35; eingegabelt rund −60), ab C♯6 kein Bend
        CHECK(wertAn(s.bendGrenzeCent, 53, Skala::Linear) < -40.0);
        if (s.noteMax >= 85) CHECK(wertAn(s.bendGrenzeCent, 85, Skala::Linear) > -2.0);
        MESSAGE(std::string(name) << ": Grenze an F3 " << wertAn(s.bendGrenzeCent, 53, Skala::Linear) << " Cent bei Oeffnung "
                     << wertAn(s.bendGrenzeOeffnung, 53, Skala::Logarithmisch) << ", Umkehrung bis " << groessterFehler << " Cent daneben");
        CHECK(nichtMonoton == 0);
        CHECK(groessterFehler < 2.5);
    }
    // ohne Bend: die Kurven flach, `pitch` laesst das Ventil offen
    for (const char* name : {"harmonium", "harmonica"})
    {
        const ZungenSpiel s = spielGestimmt(name);
        CHECK_FALSE(s.bendUeberVentil);
        CHECK(ventilFuerCent(s, (s.noteMin + s.noteMax) / 2, -50.0) == 1.0);
    }
}

TEST_CASE("InstrumentModels 34j-6 Stimmer freeReed: Melodica Bit fuer Bit wie die Datei, Bezug, Waechter fuer veraltete Werte")
{
    const std::vector<std::string> eins = gestimmteNamen();  // alle gestimmten Werte entstehen zusammen
    REQUIRE(lumi::messung::stimmerFuer(lumi::messung::stimmer(), kKernName) != nullptr);
    Werte w = werteAus(vorlage("melodica").spiel);
    const auto beginn = std::chrono::steady_clock::now();
    const lumi::messung::StimmBericht b = lumi::messung::stimmeUndNormiere(kerne(), lumi::messung::stimmer(), kKernName, w, true, true);
    const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
    MESSAGE("Melodica gestimmt und normiert in " << dauer << " s auf " << poolThreads() << " Threads (32 Tasten)");
    CHECK_MESSAGE(b.fehler.empty(), (b.fehler.empty() ? std::string() : b.fehler.front()));
    CHECK(b.gestimmt == eins);
    REQUIRE(b.mitBezug);
    CHECK(hexVon(b.bezug) == beleg().bezugLv.at("Melodica"));
    for (const std::string& n : eins)
    {
        CAPTURE(n);
        CHECK(w.kurve(n).punkte == beleg().kurven.at("Melodica").at(n));
    }
    CHECK(richteWerte(kernTyp().tafel, w).empty());
    // der Waechter (IK2-11 a): frisch gestimmt nichts veraltet, der Bezug passt
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, w).empty());
    CHECK_FALSE(lumi::messung::bezugVeraltet(kerne(), kKernName, w));
    // absichtlich veraltet: eine Kurve der Zunge geaendert — die gestimmte Kurve meldet sich
    Werte alt = w;
    Kurve d = alt.kurve("reedDamping");
    for (NotenWert& p : d.punkte) p.wert *= 1.5;
    alt.setze("reedDamping", d);
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, alt) == eins);
    // ein fremder Abdruck an einem gestimmten Wert genuegt
    Werte fremd = w;
    fremd.merkeAbdruck(std::string(kEigenfrequenz), "0000");
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, fremd) == std::vector<std::string>{std::string(kEigenfrequenz)});
    // ein falscher Bezug faellt auf
    Werte bezug = w;
    bezug.setze(std::string(kBezug), b.bezug * 1.01);
    CHECK(lumi::messung::bezugVeraltet(kerne(), kKernName, bezug));
}

TEST_CASE("InstrumentModels 34j-6 Stimmer freeReed: alle vier Bit fuer Bit wie die Datei (LUMIVIZ_34J_STIMMEN=1)")
{
    if (!variable("LUMIVIZ_34J_STIMMEN"))
    {
        MESSAGE("LUMIVIZ_34J_STIMMEN=1 stimmt alle vier Instrumente am Modell (Melodica laeuft immer, im Fall davor)");
        return;
    }
    double gesamt = 0.0;
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const auto beginn = std::chrono::steady_clock::now();
        const lumi::messung::ZungenStimmung st = lumi::messung::zungenStimmung(v.spiel);
        const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
        gesamt += dauer;
        MESSAGE(v.name << ": " << (v.spiel.noteMax - v.spiel.noteMin + 1) << " Tasten in " << dauer << " s");
        CHECK(st.stumm.empty());
        for (const auto& [n, k] : gestimmteKurven(st))
        {
            CAPTURE(n);
            CHECK(k.punkte == beleg().kurven.at(v.spiel.name).at(n));
        }
    }
    MESSAGE("alle vier in " << gesamt << " s auf " << poolThreads() << " Threads");
}

namespace
{

/// Die Lautheit des ganzen Tons (alle Choere, Gehaeuse, Bezug) bei einer Expression: 1 s gehalten, Momentary ueber
/// die letzten 0,4 s des Haltens — die Groesse des Befunds S188 (Lautheit ueber den Umfang des Instruments).
double tonLufs(const ZungenSpiel& s, int note, double e)
{
    std::vector<float> t;
    rechneZungenTon(s, ton(note, e, 1.0), t, 57600);
    std::vector<float> stereo(2 * 19200, 0.0f);
    for (std::size_t i = 0; i < 19200; ++i) stereo[2 * i] = stereo[2 * i + 1] = t[48000 - 19200 + i];
    lumi::audio::LautheitsMessung l(48000);
    l.fuettere(stereo.data(), 19200);
    return l.ergebnis().momentanMaxLufs;
}

}  // namespace

TEST_CASE("InstrumentModels 34j-6 Messfall freeReed: die Zielgroessen von 34j-5 am Kern (LUMIVIZ_34J_MESSUNG=<datei.md>)")
{
    const char* ziel = std::getenv("LUMIVIZ_34J_MESSUNG");
    if (ziel == nullptr || std::string(ziel).empty())
    {
        MESSAGE("LUMIVIZ_34J_MESSUNG=<datei.md> misst je Instrument und Taste die Zielgroessen von 34j-5 am gestimmten Kern");
        return;
    }
    std::ostringstream o;
    o << "# 34j-6 Messung am Kern freeReed (Rechenfassung 2)\n\n"
         "> test_InstrumentModelsZunge, `LUMIVIZ_34J_MESSUNG`. Je Zunge die Messkette des Pruefstands S190 (1 s gehalten, Tonhoehe aus den "
         "Nulldurchgaengen der Auslenkung, Lautheit mit AudioCore ueber die letzten 0,4 s, Einschwingen bis −5 dB der Endhuelle); gestimmt "
         "aus `werte/zunge_ic.txt`. Dazu die Lautheit des ganzen Tons (alle Choere, Gehaeuse) ueber den Umfang und der Bend ueber das "
         "Ventil.\n";
    for (const Vorlage& v : vorlagen())
    {
        const ZungenSpiel s = spielGestimmt(v.name);
        const lumi::messung::ZungenWindbereich w = lumi::messung::messeWindbereich(s);
        const double druck[3] = {s.windMinPa, windMitte(s), s.windMaxPa};
        char kopf[256];
        std::snprintf(kopf, sizeof kopf, "\n## %s — Wind %.0f / %.0f / %.0f Pa, Noten %d–%d\n\n", s.name.c_str(), druck[0], druck[1], druck[2], s.noteMin,
                      s.noteMax);
        o << kopf << "| Note | f0/f ct | min: ct | LUFS | ein ms | A/L % | Mitte: ct | LUFS | ein ms | A/L % | max: ct | LUFS | ein ms | A/L % |\n"
                     "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|\n";
        int ok[3] = {0, 0, 0};
        double lo[3] = {1e9, 1e9, 1e9}, hi[3] = {-1e9, -1e9, -1e9}, cent[3] = {0, 0, 0}, einLo = 1e9, einHi = 0, dynLo = 1e9, dynHi = -1e9;
        double steigLo = 1e9, steigHi = -1e9;
        for (std::size_t i = 0; i < w.noten.size(); ++i)
        {
            const int n = w.noten[i];
            const lumi::messung::ZungenTaste* m[3] = {&w.min[i], &w.mitte[i], &w.max[i]};
            char z[512];
            std::snprintf(z, sizeof z, "| %d | %+.1f |", n, 1200.0 * std::log2(wertAn(s.eigenfrequenzJeNote, n, Skala::Logarithmisch) / hzVon(n)));
            o << z;
            bool alle = true;
            for (int j = 0; j < 3; ++j)
            {
                const double c = centVon(m[j]->hz, n);
                std::snprintf(z, sizeof z, " %s%+.1f | %.1f | %.0f | %.1f |", m[j]->spricht ? "" : (m[j]->waechst ? "~" : "x"), c, m[j]->lufs,
                              1e3 * m[j]->einS, 100.0 * m[j]->ampLaenge);
                o << z;
                if (m[j]->spricht)
                {
                    ++ok[j];
                    lo[j] = std::min(lo[j], m[j]->lufs);
                    hi[j] = std::max(hi[j], m[j]->lufs);
                    cent[j] = std::max(cent[j], std::abs(c));
                }
                else
                    alle = false;
            }
            o << "\n";
            if (alle)
            {
                dynLo = std::min(dynLo, m[2]->lufs - m[0]->lufs);
                dynHi = std::max(dynHi, m[2]->lufs - m[0]->lufs);
                einLo = std::min(einLo, m[1]->einS);
                einHi = std::max(einHi, m[1]->einS);
                const double steig = 1200.0 * std::log2(m[2]->hz / m[0]->hz) / (1e-3 * (druck[2] - druck[0]));
                steigLo = std::min(steigLo, steig);
                steigHi = std::max(steigHi, steig);
            }
        }
        char zz[640];
        std::snprintf(zz, sizeof zz,
                      "\n**ZIEL %s:** spricht min/mitte/max %d/%d/%d von %zu | Lautheit ueber den Umfang %.1f / %.1f / %.1f LU | groesste "
                      "Abweichung %.1f / %.1f / %.1f ct | Dynamik %.1f … %.1f LU | Einschwingen Mitte %.0f … %.0f ms | Tonhoehe ueber Druck "
                      "%.1f … %.1f ct/kPa\n",
                      s.name.c_str(), ok[0], ok[1], ok[2], w.noten.size(), hi[0] - lo[0], hi[1] - lo[1], hi[2] - lo[2], cent[0], cent[1], cent[2],
                      dynLo, dynHi, 1e3 * einLo, 1e3 * einHi, steigLo, steigHi);
        o << zz;
        MESSAGE(zz);
        // die Lautheit des ganzen Tons ueber den Umfang (Befund S188), jede Taste
        const int anzahl = s.noteMax - s.noteMin + 1;
        std::vector<double> l(static_cast<std::size_t>(3 * anzahl));
        parallelJe(l.size(), [&](std::size_t k) { l[k] = tonLufs(s, s.noteMin + static_cast<int>(k / 3), 0.5 * static_cast<double>(k % 3)); });
        double tlo[3] = {1e9, 1e9, 1e9}, thi[3] = {-1e9, -1e9, -1e9};
        int still = 0;
        for (std::size_t k = 0; k < l.size(); ++k)
        {
            if (l[k] <= lumi::audio::kLufsStille + 1.0)
            {
                ++still;
                continue;
            }
            tlo[k % 3] = std::min(tlo[k % 3], l[k]);
            thi[k % 3] = std::max(thi[k % 3], l[k]);
        }
        std::snprintf(zz, sizeof zz, "**Ton %s** (alle Choere, Gehaeuse, Bezug): Lautheit ueber den Umfang bei Expression 0 / 0,5 / 1: %.1f / %.1f / %.1f LU, "
                      "%d still\n",
                      s.name.c_str(), thi[0] - tlo[0], thi[1] - tlo[1], thi[2] - tlo[2], still);
        o << zz;
        MESSAGE(zz);
    }
    // der Bend ueber das Ventil (S190 §7): am mittleren Druck, gestimmt mit offenem Ventil; Cent gegen offen, dB gegen offen
    const std::vector<double> faktor = {1.0, 0.7, 0.5, 0.35, 0.25, 0.18, 0.12};
    for (const char* name : {"accordion", "melodica"})
    {
        const ZungenSpiel s = spielGestimmt(name);
        o << "\n## Bend " << s.name << " — Ventilflaeche mal Faktor am mittleren Druck; je Feld Cent / dB gegen offen (x = spricht nicht)\n\n| Note |";
        for (const double f : faktor) o << " " << f << " |";
        o << "\n|---|";
        for (std::size_t i = 0; i < faktor.size(); ++i) o << "---|";
        o << "\n";
        const int von = std::max(s.noteMin, s.bassAb > 0 ? s.bassAb : s.noteMin);
        std::vector<int> noten;
        for (int n = von; n <= s.noteMax; ++n) noten.push_back(n);
        std::vector<lumi::messung::ZungenTaste> m(noten.size() * faktor.size());
        parallelJe(m.size(), [&](std::size_t k) {
            const int n = noten[k / faktor.size()];
            m[k] = lumi::messung::messeZungenTaste(s, wertAn(s.eigenfrequenzJeNote, n, Skala::Logarithmisch), n, windMitte(s), faktor[k % faktor.size()]);
        });
        int nichtMonoton = 0;
        for (std::size_t i = 0; i < noten.size(); ++i)
        {
            const lumi::messung::ZungenTaste& offen = m[i * faktor.size()];
            o << "| " << noten[i] << " |";
            double vorher = 1e9;
            for (std::size_t j = 0; j < faktor.size(); ++j)
            {
                const lumi::messung::ZungenTaste& x = m[i * faktor.size() + j];
                const double c = x.hz > 0.0 && offen.hz > 0.0 ? 1200.0 * std::log2(x.hz / offen.hz) : 0.0;
                char z[64];
                std::snprintf(z, sizeof z, " %s%+.0f / %+.1f |", x.spricht ? "" : "x", c, x.lufs - offen.lufs);
                o << z;
                if (x.spricht)
                {
                    if (c > vorher + 0.5) ++nichtMonoton;
                    vorher = c;
                }
            }
            o << "\n";
        }
        o << "\nnicht monoton (ein kleineres Ventil hoeher als das groessere, ueber 0,5 Cent): " << nichtMonoton << "\n";
        MESSAGE(s.name << ": Bend nicht monoton an " << nichtMonoton << " Stellen");

        // Wie genau trifft eine Ablage der Kurve Oeffnung → Cent den Bend? Wahrheit: Raster 2^(−1/8) bis zur Grenze,
        // die Grenze (letzte Oeffnung, die anspricht) auf 1 % genau eingegabelt. Abgelegt: (1; 0), die Rasterpunkte
        // ueber der Grenze und die Grenze selbst; dazwischen linear in ln(Oeffnung).
        struct Ablage
        {
            const char* name;
            std::vector<double> raster;
        };
        std::vector<Ablage> ablagen = {{"0,7 / 0,5 / 0,35 und Grenze", {0.7, 0.5, 0.35}}, {"2^(-1/4) und Grenze", {}}, {"nur Grenze", {}}};
        for (int k = 1; k <= 12; ++k) ablagen[1].raster.push_back(std::exp2(-k / 4.0));
        std::vector<double> groesster(ablagen.size(), 0.0), grenzeCent(noten.size(), 0.0), grenzeOeffnung(noten.size(), 1.0);
        std::vector<std::vector<double>> fehler(noten.size(), std::vector<double>(ablagen.size(), 0.0));
        parallelJe(noten.size(), [&](std::size_t i) {
            const int n = noten[i];
            const double f0 = wertAn(s.eigenfrequenzJeNote, n, Skala::Logarithmisch);
            const auto miss = [&](double f) { return lumi::messung::messeZungenTaste(s, f0, n, windMitte(s), f); };
            const double offenHz = miss(1.0).hz;
            std::vector<std::pair<double, double>> wahr = {{1.0, 0.0}};
            double letzte = 1.0, erste = 0.0;
            for (int k = 1; k <= 32; ++k)
            {
                const double f = std::exp2(-k / 8.0);
                const lumi::messung::ZungenTaste x = miss(f);
                if (!x.spricht)
                {
                    erste = f;
                    break;
                }
                wahr.emplace_back(f, 1200.0 * std::log2(x.hz / offenHz));
                letzte = f;
            }
            double gutF = letzte, gutC = wahr.back().second;
            for (int r = 0; r < 12 && erste > 0.0 && gutF / erste > 1.01; ++r)
            {
                const double mitte = std::sqrt(gutF * erste);
                const lumi::messung::ZungenTaste x = miss(mitte);
                if (x.spricht)
                {
                    gutF = mitte;
                    gutC = 1200.0 * std::log2(x.hz / offenHz);
                }
                else
                    erste = mitte;
            }
            grenzeOeffnung[i] = gutF;
            grenzeCent[i] = gutC;
            for (std::size_t a = 0; a < ablagen.size(); ++a)
            {
                std::vector<std::pair<double, double>> p = {{1.0, 0.0}};
                for (const double f : ablagen[a].raster)
                    if (f > gutF)
                    {
                        const auto it = std::find_if(wahr.begin(), wahr.end(), [&](const auto& w) { return std::abs(w.first / f - 1.0) < 1e-9; });
                        p.emplace_back(f, it != wahr.end() ? it->second : 1200.0 * std::log2(miss(f).hz / offenHz));
                    }
                p.emplace_back(gutF, gutC);
                for (const auto& [f, c] : wahr)
                {
                    std::size_t j = 1;
                    while (j + 1 < p.size() && p[j].first > f) ++j;
                    const double u = (std::log(f) - std::log(p[j - 1].first)) / (std::log(p[j].first) - std::log(p[j - 1].first));
                    fehler[i][a] = std::max(fehler[i][a], std::abs(p[j - 1].second + u * (p[j].second - p[j - 1].second) - c));
                }
            }
        });
        o << "\n### Ablage der Kurve Oeffnung → Cent: groesster Fehler gegen das Raster 2^(-1/8) (Cent)\n\n| Note | Grenze Oeffnung | Grenze Cent |";
        for (const Ablage& a : ablagen) o << " " << a.name << " |";
        o << "\n|---|---|---|---|---|---|\n";
        for (std::size_t i = 0; i < noten.size(); ++i)
        {
            char z[200];
            std::snprintf(z, sizeof z, "| %d | %.3f | %+.1f | %.2f | %.2f | %.2f |\n", noten[i], grenzeOeffnung[i], grenzeCent[i], fehler[i][0], fehler[i][1],
                          fehler[i][2]);
            o << z;
            for (std::size_t a = 0; a < ablagen.size(); ++a) groesster[a] = std::max(groesster[a], fehler[i][a]);
        }
        o << "\ngroesster Fehler:";
        for (std::size_t a = 0; a < ablagen.size(); ++a) o << " " << ablagen[a].name << " " << groesster[a] << " ct;";
        o << "\n";
        MESSAGE(s.name << ": Fehler der Ablage " << groesster[0] << " / " << groesster[1] << " / " << groesster[2] << " Cent");
    }
    // 34j-7: der Pegel ueber der Staerke (Stufe → Staerke, SK4 a, TM1 a) und der Ausklang nach dem Wind (Nachklang)
    o << "\n## 34j-7 Pegel ueber der Staerke — ganzer Ton, Lautheit gegen Staerke 1 (LU), jede vierte Taste\n";
    for (const Vorlage& v : vorlagen())
    {
        const ZungenSpiel s = spielGestimmt(v.name);
        std::vector<int> noten;
        for (int n = s.noteMin; n <= s.noteMax; n += 4) noten.push_back(n);
        constexpr int kStufen = 11;
        std::vector<double> l(noten.size() * kStufen);
        parallelJe(l.size(), [&](std::size_t k) { l[k] = tonLufs(s, noten[k / kStufen], static_cast<double>(k % kStufen) / 10.0); });
        std::vector<double> mittel(kStufen, 0.0), lo(kStufen, 1e9), hi(kStufen, -1e9);
        for (std::size_t i = 0; i < noten.size(); ++i)
            for (int j = 0; j < kStufen; ++j)
            {
                const double d = l[i * kStufen + j] - l[i * kStufen + kStufen - 1];
                mittel[j] += d / static_cast<double>(noten.size());
                lo[j] = std::min(lo[j], d);
                hi[j] = std::max(hi[j], d);
            }
        const double dyn = -mittel[0];
        double abweichung = 0.0;
        for (int j = 0; j < kStufen; ++j) abweichung = std::max(abweichung, std::abs(mittel[j] + dyn * (1.0 - j / 10.0)));
        o << "\n### " << s.name << "\n\n| Staerke | Mittel | von | bis |\n|---|---|---|---|\n";
        for (int j = 0; j < kStufen; ++j)
        {
            char z[120];
            std::snprintf(z, sizeof z, "| %.1f | %+.1f | %+.1f | %+.1f |\n", j / 10.0, mittel[j], lo[j], hi[j]);
            o << z;
        }
        // die Stufe, an der die gemessene Kurve den Pegel trifft, gegen die Gerade s = 1 + Stufe/Dynamik
        o << "\nDynamik (Staerke 0 → 1) " << dyn << " LU; groesste Abweichung der Kurve von der Geraden " << abweichung << " LU\n\n| Stufe | Staerke aus der Kurve | aus der Geraden |\n|---|---|---|\n";
        for (const double stufe : {-3.0, -6.0, -12.0, -20.0, -30.0})
        {
            double sk = 0.0;
            for (int j = kStufen - 1; j > 0; --j)
                if (mittel[j - 1] <= stufe && stufe <= mittel[j])
                {
                    sk = (j - 1 + (stufe - mittel[j - 1]) / (mittel[j] - mittel[j - 1])) / 10.0;
                    break;
                }
            char z[120];
            std::snprintf(z, sizeof z, "| %.0f | %.3f | %.3f |\n", stufe, sk, std::max(0.0, 1.0 + stufe / dyn));
            o << z;
        }
        MESSAGE(s.name << ": Dynamik " << dyn << " LU, Abweichung von der Geraden " << abweichung << " LU");
    }
    // Drei Abbildungen Stufe → Staerke gegen das Ziel „eine Note bei −x dB klingt x LU leiser, hoechstens bis zur
    // Dynamik der Taste“, am Ton nachgemessen: a je Taste an ihrer Kurve (Raster 2 dB, dazwischen linear), b an der
    // mittleren Kurve des Instruments, c die Gerade s = 1 + Stufe/Dynamik
    o << "\n## 34j-7 Stufe → Staerke: Fehler gegen das Ziel (LU), am Ton nachgemessen, jede zweite Taste\n\n"
         "| Instrument | a je Taste, Raster 2 dB | b mittlere Kurve | c Gerade |\n|---|---|---|---|\n";
    for (const Vorlage& v : vorlagen())
    {
        const ZungenSpiel s = spielGestimmt(v.name);
        std::vector<int> noten;
        for (int n = s.noteMin; n <= s.noteMax; n += 2) noten.push_back(n);
        constexpr int kRaster = 21;  // Staerke 0, 0,05 … 1
        std::vector<double> kurve(noten.size() * kRaster);
        parallelJe(kurve.size(), [&](std::size_t k) { kurve[k] = tonLufs(s, noten[k / kRaster], static_cast<double>(k % kRaster) / 20.0); });
        // relativ zu Staerke 1; die Staerke zu einem Pegel: von oben die erste Stelle, an der die Kurve ihn erreicht
        const auto staerkeFuer = [](const std::vector<double>& l, double ziel) {
            if (ziel >= 0.0) return 1.0;
            for (int j = kRaster - 1; j > 0; --j)
                if (l[static_cast<std::size_t>(j - 1)] <= ziel && ziel <= l[static_cast<std::size_t>(j)])
                {
                    const double u = (ziel - l[static_cast<std::size_t>(j - 1)]) / (l[static_cast<std::size_t>(j)] - l[static_cast<std::size_t>(j - 1)]);
                    return (j - 1 + u) / 20.0;
                }
            return 0.0;
        };
        std::vector<std::vector<double>> rel(noten.size(), std::vector<double>(kRaster));
        std::vector<double> mittel(kRaster, 0.0);
        for (std::size_t i = 0; i < noten.size(); ++i)
            for (int j = 0; j < kRaster; ++j)
            {
                rel[i][static_cast<std::size_t>(j)] = kurve[i * kRaster + j] - kurve[i * kRaster + kRaster - 1];
                mittel[static_cast<std::size_t>(j)] += rel[i][static_cast<std::size_t>(j)] / static_cast<double>(noten.size());
            }
        const double dyn = -mittel[0];
        const std::vector<double> stufen = {-1.0, -3.0, -5.0, -7.0, -11.0, -15.0, -19.0};
        std::vector<double> soll(noten.size() * stufen.size()), sk[3];
        for (auto& x : sk) x.resize(soll.size());
        for (std::size_t i = 0; i < noten.size(); ++i)
            for (std::size_t j = 0; j < stufen.size(); ++j)
            {
                const double st = stufen[j];
                const std::size_t k = i * stufen.size() + j;
                soll[k] = std::max(st, rel[i].front());
                // a: Raster 2 dB je Taste, dazwischen linear
                const double unten = std::floor(st / 2.0) * 2.0, oben = unten + 2.0;
                const double su = staerkeFuer(rel[i], unten), so = staerkeFuer(rel[i], oben);
                sk[0][k] = su + (st - unten) / 2.0 * (so - su);
                sk[1][k] = staerkeFuer(mittel, st);
                sk[2][k] = std::max(0.0, 1.0 + st / dyn);
            }
        std::vector<double> ist(3 * soll.size());
        parallelJe(ist.size(), [&](std::size_t k) {
            const std::size_t a = k / soll.size(), m = k % soll.size();
            const std::size_t i = m / stufen.size();
            ist[k] = tonLufs(s, noten[i], sk[a][m]) - kurve[i * kRaster + kRaster - 1];
        });
        double fehler[3] = {0, 0, 0};
        for (std::size_t k = 0; k < ist.size(); ++k) fehler[k / soll.size()] = std::max(fehler[k / soll.size()], std::abs(ist[k] - soll[k % soll.size()]));
        char z[200];
        std::snprintf(z, sizeof z, "| %s | %.2f | %.2f | %.2f |\n", s.name.c_str(), fehler[0], fehler[1], fehler[2]);
        o << z;
        MESSAGE(s.name << ": Stufe → Staerke, groesster Fehler a " << fehler[0] << " / b " << fehler[1] << " / c " << fehler[2] << " LU");
    }
    // die Helligkeit ueber der Staerke: Schwerpunkt der ersten zwanzig Teiltoene bei Staerke 0 und 1 (ganzer Ton)
    o << "\n## 34j-7 Helligkeit ueber der Staerke — Schwerpunkt der Teiltoene 1–20, Staerke 1 gegen 0\n\n| Instrument | Verhaeltnis min … max | Tasten heller |\n|---|---|---|\n";
    for (const Vorlage& v : vorlagen())
    {
        const ZungenSpiel s = spielGestimmt(v.name);
        std::vector<int> noten;
        for (int n = s.noteMin; n <= s.noteMax; n += 4) noten.push_back(n);
        std::vector<double> sp(2 * noten.size());
        parallelJe(sp.size(), [&](std::size_t k) {
            const int n = noten[k / 2];
            std::vector<float> t;
            rechneZungenTon(s, ton(n, k % 2 == 0 ? 0.0 : 1.0, 1.0), t, 57600);
            const double f0 = hzVon(n);
            double summe = 0.0, gewicht = 0.0;
            for (int h = 1; h <= 20 && h * f0 < 20000.0; ++h)
            {
                const double b = band(t, 28800, 9600, h * f0);
                summe += h * f0 * b;
                gewicht += b;
            }
            sp[k] = gewicht > 0.0 ? summe / gewicht : 0.0;
        });
        double lo = 1e9, hi = 0.0;
        int heller = 0;
        for (std::size_t i = 0; i < noten.size(); ++i)
        {
            const double r = sp[2 * i + 1] / sp[2 * i];
            lo = std::min(lo, r);
            hi = std::max(hi, r);
            heller += r > 1.0 ? 1 : 0;
        }
        char z[200];
        std::snprintf(z, sizeof z, "| %s | %.2f … %.2f | %d von %zu |\n", s.name.c_str(), lo, hi, heller, noten.size());
        o << z;
        MESSAGE(s.name << ": Schwerpunkt ff/pp " << lo << " … " << hi << ", heller an " << heller << " von " << noten.size());
    }
    o << "\n## 34j-7 Ausklang nach dem Wind — bis −60 dB des gehaltenen Tons (Expression 0,5, ganzer Ton), gegen 6,9 / (ζ·ω₀)\n\n"
         "| Instrument | gemessen min … max (ms) | Formel min … max (ms) | ueber 1 s |\n|---|---|---|---|\n";
    for (const Vorlage& v : vorlagen())
    {
        const ZungenSpiel s = spielGestimmt(v.name);
        const int anzahl = s.noteMax - s.noteMin + 1;
        std::vector<double> gemessen(static_cast<std::size_t>(anzahl)), formel(static_cast<std::size_t>(anzahl));
        std::vector<double> beiPp(static_cast<std::size_t>(anzahl)), beiFf(static_cast<std::size_t>(anzahl));
        const auto ausklang = [&](int n, double e) {
            std::vector<float> t;
            rechneZungenTon(s, ton(n, e, 0.5), t, 48000 * 21 / 2);
            const std::size_t block = 240;
            double ref = 0.0;
            for (std::size_t k = 19200; k < 24000; ++k) ref += double(t[k]) * double(t[k]);
            ref /= 4800.0;
            std::size_t letzter = 24000;
            for (std::size_t k = 24000; k + block <= t.size(); k += block)
            {
                double x = 0.0;
                for (std::size_t j = 0; j < block; ++j) x += double(t[k + j]) * double(t[k + j]);
                if (x / block > ref * 1e-6) letzter = k + block;
            }
            return 1e3 * static_cast<double>(letzter - 24000) / 48000.0;
        };
        parallelJe(gemessen.size(), [&](std::size_t i) {
            const int n = s.noteMin + static_cast<int>(i);
            beiPp[i] = ausklang(n, 0.0);
            beiFf[i] = ausklang(n, 1.0);
            std::vector<float> t;
            rechneZungenTon(s, ton(n, 0.5, 0.5), t, 48000 * 21 / 2);
            const std::size_t block = 240;
            double ref = 0.0;
            for (std::size_t k = 19200; k < 24000; ++k) ref += double(t[k]) * double(t[k]);
            ref /= 4800.0;
            std::size_t letzter = 24000;
            for (std::size_t k = 24000; k + block <= t.size(); k += block)
            {
                double e = 0.0;
                for (std::size_t j = 0; j < block; ++j) e += double(t[k + j]) * double(t[k + j]);
                if (e / block > ref * 1e-6) letzter = k + block;
            }
            gemessen[i] = 1e3 * static_cast<double>(letzter - 24000) / 48000.0;
            const double f0 = wertAn(s.eigenfrequenzJeNote, n, Skala::Logarithmisch);
            formel[i] = 1e3 * 6.9078 / (zungeAn(s, n).zeta * 2.0 * kPi * f0);
        });
        int ueber = 0;
        for (const double g : gemessen) ueber += g > 1000.0 ? 1 : 0;
        char z[200];
        std::snprintf(z, sizeof z, "| %s | %.0f … %.0f | %.0f … %.0f | %d von %d |\n", s.name.c_str(), *std::min_element(gemessen.begin(), gemessen.end()),
                      *std::max_element(gemessen.begin(), gemessen.end()), *std::min_element(formel.begin(), formel.end()),
                      *std::max_element(formel.begin(), formel.end()), ueber, anzahl);
        o << z;
        MESSAGE(z);
        double vonPp = 1e9, bisPp = 0.0, vonFf = 1e9, bisFf = 0.0, anteilLo = 1e9, anteilHi = 0.0;
        for (std::size_t i = 0; i < gemessen.size(); ++i)
        {
            vonPp = std::min(vonPp, beiPp[i] / gemessen[i]);
            bisPp = std::max(bisPp, beiPp[i] / gemessen[i]);
            vonFf = std::min(vonFf, beiFf[i] / gemessen[i]);
            bisFf = std::max(bisFf, beiFf[i] / gemessen[i]);
            anteilLo = std::min(anteilLo, gemessen[i] / formel[i]);
            anteilHi = std::max(anteilHi, gemessen[i] / formel[i]);
        }
        char zz0[240];
        std::snprintf(zz0, sizeof zz0, "%s: Ausklang bei Expression 0 / 1 gegen 0,5: %.2f … %.2f / %.2f … %.2f; gemessen gegen die Formel %.2f … %.2f",
                      s.name.c_str(), vonPp, bisPp, vonFf, bisFf, anteilLo, anteilHi);
        o << "\n" << zz0 << "\n";
        MESSAGE(zz0);
        o << "\n<details><summary>je Taste (" << s.name << ")</summary>\n\n| Note | gemessen ms | Expression 0 | Expression 1 | Formel ms |\n|---|---|---|---|---|\n";
        for (int i = 0; i < anzahl; i += 3)
        {
            const auto k = static_cast<std::size_t>(i);
            char zz[120];
            std::snprintf(zz, sizeof zz, "| %d | %.0f | %.0f | %.0f | %.0f |\n", s.noteMin + i, gemessen[k], beiPp[k], beiFf[k], formel[k]);
            o << zz;
        }
        o << "\n</details>\n";
    }
    std::ofstream f(ziel, std::ios::binary);
    f << o.str();
    CHECK(f.good());
    MESSAGE("geschrieben: " << std::string(ziel));
}
