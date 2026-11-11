/**
 ****************************************************************************************
 * @file   test_InstrumentModelsZunge.cpp
 * @brief  InstrumentModels, Stufe 34j: Klasse 10 (durchschlagende Zunge) — der Kern `freeReed`
 *         (docs/audio/Instrumente_IC_Konzept.md 34j-2, ZV1 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine.
 *
 * - Tafel, Fabrik, Vorlagen, Werte hin und zurueck.
 * - Die Abdruecke aus `werte/zunge_ic.txt` (IC-Vorlage, `docs/modellierung/anbindung-ic/abdruck_zunge.cpp`) Bit fuer Bit —
 *   mit der Stimmung und dem Bezug aus derselben Datei (der Stimmer kommt mit 34j-3), ueber den direkten Aufruf und, wo der
 *   Adapter hinreicht, ueber den Kern.
 * - Der Bezug: `kern::normiereNachTafel` gleich den Zeilen `bezugLv` (die IC-Rechnung mit der Probe von LumiViz), und die
 *   IC-Rechnung `normiere` am uebernommenen Kern gleich den Zeilen `bezug`.
 * - Die IC-Faelle aus `rechnung/klassen/10_zunge/test_zunge.cpp` (`4409088`) mit den IC-Grenzen; gemessen wird mit
 *   `InstrumentMeasure/ZungeStimmen` (IK2-2 a). Die Rechenzeit kommt mit 34j-9 (`*34j Rechenzeit*`).
 * - Der Stimmer (34j-3): die Melodica ueber `stimmeUndNormiere` Bit fuer Bit gleich IC `Stimmung.inc`, mit Bezug und dem
 *   Waechter fuer veraltete Werte; alle vier Instrumente nur mit `LUMIVIZ_34J_STIMMEN=1` (rechnet je Instrument einige
 *   zehn Sekunden), ohne Variable nur eine Meldung.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Pool.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentMeasure/ZungeStimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/zunge/ZungeKern.hpp"

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

/// Der Beleg aus IC (`werte/zunge_ic.txt`).
struct IcBeleg
{
    std::map<std::string, std::string> abdruck;
    std::map<std::string, std::string> bezug, bezugLv;
    /// je IC-Name des Instruments und Kurve die Punkte
    std::map<std::string, std::map<std::string, std::vector<NotenWert>>> kurven;
};

const IcBeleg& icBeleg()
{
    static const IcBeleg b = [] {
        IcBeleg r;
        std::ifstream f(std::filesystem::path(__FILE__).parent_path() / "werte" / "zunge_ic.txt");
        std::string zeile;
        while (std::getline(f, zeile))
        {
            std::istringstream z(zeile);
            std::string art, name;
            z >> art >> name;
            if (art == "abdruck" || art == "bezug" || art == "bezugLv")
            {
                std::string wert;
                z >> wert;
                (art == "abdruck" ? r.abdruck : art == "bezug" ? r.bezug : r.bezugLv)[name] = wert;
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
    }();
    return b;
}

const Vorlage& vorlage(std::string_view name)
{
    for (const Vorlage& v : vorlagen())
        if (v.name == name) return v;
    FAIL("keine Vorlage " << name);
    return vorlagen().front();
}

/// Die Werte einer Vorlage mit der Stimmung aus dem Beleg (IC `Stimmung.inc`); mit `icBezug` der IC-Bezug, sonst 1.
Werte gestimmteWerte(const Vorlage& v, bool icBezug)
{
    Werte w = werteAus(v.spiel);
    const auto& k = icBeleg().kurven.at(v.spiel.name);
    const auto kurve = [&](std::string_view welche) {
        Kurve c;
        c.punkte = k.at(std::string(welche));
        c.belegkraft = Belegkraft::Rechnung;
        c.herkunft = "IC Stimmung.inc (werte/zunge_ic.txt)";
        return c;
    };
    w.setze(std::string(kEigenfrequenz), kurve("eigenfrequenz"));
    w.setze(std::string(kDruckUnten), kurve("druckUnten"));
    w.setze(std::string(kDruckOben), kurve("druckOben"));
    w.setze(std::string(kBezug), icBezug ? ausHex(icBeleg().bezug.at(v.spiel.name)) : 1.0);
    const std::vector<std::string> f = richteWerte(kernTyp().tafel, w);
    REQUIRE_MESSAGE(f.empty(), v.name << ": " << (f.empty() ? std::string() : f.front()));
    return w;
}

/// Das Spiel wie IC (gestimmt, mit IC-Bezug) — ueber die Tafel, wie die App es baut.
ZungenSpiel spielWieIc(std::string_view name)
{
    ZungenSpiel s = spielAus(gestimmteWerte(vorlage(name), true));
    s.name = vorlage(name).spiel.name;
    return s;
}

const ZungenSpiel& akkordeon()
{
    static const ZungenSpiel a = spielWieIc("accordion");
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

/// Ein Ton ueber den Kern (Rechenweg Note, beginn 0) mit den Werten wie IC.
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

}  // namespace

TEST_CASE("InstrumentModels 34j-2 Kern freeReed: Tafel, Fabrik, Vorlagen, Werte hin und zurueck")
{
    const KernTyp& t = kernTyp();
    const std::vector<std::string> fehler = pruefeTafel(t.tafel);
    CHECK_MESSAGE(fehler.empty(), (fehler.empty() ? std::string() : fehler.front()));
    REQUIRE(kerne().typ(kKernName) != nullptr);
    CHECK(t.tafel.bezugWert == kBezug);
    CHECK(t.tafel.umfangVon == "noteMin");
    CHECK(t.tafel.umfangBis == "noteMax");
    CHECK(t.tafel.gruppen == std::vector<std::string>{"Keys", "Reed", "Chamber", "Wind", "Ranks", "Bass ranks", "Case", "Tunable", "Output"});
    CHECK(t.auskunft.rechenweg == Rechenweg::Note);
    CHECK_FALSE(t.auskunft.koerperAnDerSchicht);
    CHECK(t.auskunft.spuren == std::vector<std::string>{"expression", "pitch", "vibrato"});
    CHECK(t.auskunft.innere == std::vector<std::string>{std::string(kInnereSpitze)});
    std::vector<std::string> gestimmt;
    for (const ParameterAngabe& a : t.tafel.werte)
    {
        CAPTURE(a.name);
        CHECK_FALSE(a.quellen.empty());
        CHECK_FALSE(a.herkunft.empty());
        if (a.gestimmt) gestimmt.push_back(a.name);
    }
    CHECK(gestimmt == std::vector<std::string>{std::string(kEigenfrequenz), std::string(kDruckUnten), std::string(kDruckOben)});

    std::vector<std::string> namen;
    for (const Vorlage& v : vorlagen()) namen.push_back(v.name);
    CHECK(namen == std::vector<std::string>{"accordion", "harmonium", "melodica", "harmonica"});
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        // ungestimmt fehlen die drei gestimmten Werte (IK2-11 a: ein Fehler, kein stiller Ersatz)
        Werte roh = werteAus(v.spiel);
        CHECK(richteWerte(t.tafel, roh).size() == 3);
        // gestimmt: die Vorgaben liegen in den Grenzen, Richten aendert nichts, zurueck ins Spiel und wieder in Werte gleich
        Werte w = werteAus(v.spiel);
        const Werte g = gestimmteWerte(v, true);
        for (const std::string_view n : {kEigenfrequenz, kDruckUnten, kDruckOben, kBezug}) w.setze(std::string(n), *g.finde(n));
        const Werte vorher = w;
        const std::vector<std::string> f = richteWerte(t.tafel, w);
        CHECK_MESSAGE(f.empty(), (f.empty() ? std::string() : f.front()));
        CHECK(w == vorher);
        CHECK(werteAus(spielAus(w)) == w);
    }
    // die Choere als Listen: das Akkordeon Musette und Bass-Seite
    const ZungenSpiel& a = akkordeon();
    REQUIRE(a.choere.size() == 2);
    CHECK(a.choere[1].schwebungA4Hz == 2.0);
    REQUIRE(a.bassChoere.size() == 2);
    CHECK(a.bassChoere[0].halbtoene == -12);
    CHECK(a.bassAb == 53);
}

TEST_CASE("InstrumentModels 34j-2 Kern freeReed: die Abdruecke aus werte/zunge_ic.txt Bit fuer Bit")
{
    const IcBeleg& b = icBeleg();
    REQUIRE(b.abdruck.size() == 49);
    int geprueft = 0;
    const auto pruefe = [&](const std::string& name, const std::string& ist) {
        CAPTURE(name);
        REQUIRE(b.abdruck.count(name) == 1);
        CHECK(ist == b.abdruck.at(name));
        ++geprueft;
    };
    // die Faelle des Belegs, direkt gerechnet (der Weg von abdruck_zunge.cpp)
    pruefe("zunge_Akkordeon_m69_mf", abdruckVon(akkordeon(), ton(69, 0.5, 0.8)));
    const struct
    {
        const char* name;
        double e;
    } stufen[] = {{"e0", 0.0}, {"e05", 0.5}, {"e1", 1.0}};
    for (const Vorlage& v : vorlagen())
    {
        const ZungenSpiel s = spielWieIc(v.name);
        const int tief = std::max(s.noteMin, s.bassAb);
        const int mitte = (tief + s.noteMax) / 2;
        for (const int n : {tief, mitte, s.noteMax})
            for (const auto& st : stufen) pruefe("zunge_" + s.name + "_m" + std::to_string(n) + "_" + st.name, abdruckVon(s, ton(n, st.e, 0.4)));
        ZungenAnschlag vib = ton(mitte, 0.5, 0.4);
        vib.vibratoCent = 20.0;
        vib.vibratoHz = 5.5;
        pruefe("zunge_" + s.name + "_m" + std::to_string(mitte) + "_vibrato", abdruckVon(s, vib));
        ZungenAnschlag bend = ton(mitte, 0.5, 0.4);
        bend.pitchCent = -50.0;
        pruefe("zunge_" + s.name + "_m" + std::to_string(mitte) + "_bend", abdruckVon(s, bend));
    }
    for (const int n : {29, 40, 52}) pruefe("zunge_Akkordeon_bass_m" + std::to_string(n) + "_e05", abdruckVon(akkordeon(), ton(n, 0.5, 0.4)));
    ZungenAnschlag w = ton(69, 0.5, 0.4);
    w.balgwechselS = 0.2;
    pruefe("zunge_Akkordeon_m69_balgwechsel", abdruckVon(akkordeon(), w));
    CHECK(geprueft == 49);

    // ueber den Kern: Expression, Bend und Bass-Seite als Spuren am Beginn (Stufe 0)
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
            for (const auto& st : stufen) CHECK(ueberKern(v, n, st.e, 19200) == b.abdruck.at(basis + std::to_string(n) + "_" + st.name));
        CHECK(ueberKern(v, mitte, 0.5, 19200, {spurAm(spur::kPitch, -50.0)}) == b.abdruck.at(basis + std::to_string(mitte) + "_bend"));
    }
    CHECK(ueberKern(vorlage("accordion"), 40, 0.5, 19200) == b.abdruck.at("zunge_Akkordeon_bass_m40_e05"));
    // ohne Spur `expression` die volle Staerke (SK4 a)
    CHECK(kernAbdruck("melodica", 68, 19200, {}) == b.abdruck.at("zunge_Melodica_m68_e1"));
    // das Vibrato des Kerns: 5 Hz (IC-Vorgabe), sonst wie der direkte Aufruf
    {
        ZungenAnschlag vib = ton(68, 0.5, 0.4);
        vib.vibratoCent = 20.0;
        CHECK(ueberKern(vorlage("melodica"), 68, 0.5, 19200, {spurAm(spur::kVibrato, 20.0)}) == abdruckVon(spielWieIc("melodica"), vib));
    }
}

TEST_CASE("InstrumentModels 34j-2 Kern freeReed: der Bezug nach der Tafel und nach IC")
{
    const IcBeleg& b = icBeleg();
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        // die Probe von LumiViz (IK2-10 a): gleich der IC-Rechnung mit dieser Probe
        std::vector<std::string> fehler;
        const double lv = normiereNachTafel(kerne(), kKernName, gestimmteWerte(v, false), &fehler);
        REQUIRE_MESSAGE(fehler.empty(), (fehler.empty() ? std::string() : fehler.front()));
        CHECK(hexVon(lv) == b.bezugLv.at(v.spiel.name));
        // IC normiere am uebernommenen Kern: 0,6 s, Expression 1, an der Mitte ab der Bass-Seite
        ZungenSpiel s = spielAus(gestimmteWerte(v, false));
        ZungenAnschlag a = ton((std::max(s.noteMin, s.bassAb) + s.noteMax) / 2, 1.0, 0.6);
        std::vector<float> t;
        rechneZungenTon(s, a, t, static_cast<std::int64_t>(0.6 * a.rate));
        float sp = 0.0f;
        for (const float x : t) sp = std::max(sp, std::abs(x));
        const double ic = sp > 0.0f ? sp / 0.5 : 1.0;
        CHECK(hexVon(ic) == b.bezug.at(v.spiel.name));
        MESSAGE(v.name << ": Bezug IC " << ic << ", LumiViz " << lv);
    }
}

TEST_CASE("InstrumentModels 34j-2 IC test_zunge: spricht selbst an und schwingt etwas unter f0, klingt nach dem Wind aus")
{
    const ZungenSpiel& s = akkordeon();
    const double f0 = wertAn(s.eigenfrequenzJeNote, 60, Skala::Logarithmisch);
    std::vector<double> t;
    const lumi::messung::ZungenTon m = lumi::messung::messeEineZunge(s, f0, hzVon(60), ton(60, 0.5), 48000, &t);
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

TEST_CASE("InstrumentModels 34j-2 IC test_zunge: Akkordeon jede dritte Diskantnote bei pp, mf, ff auf ±10 Cent, ff schneller da")
{
    const ZungenSpiel& s = akkordeon();
    int getroffen = 0, gesamt = 0;
    for (int note = s.bassAb; note <= s.noteMax; note += 3)
    {
        double einPp = 0.0, einFf = 0.0;
        for (const double e : {0.0, 0.5, 1.0})
        {
            const lumi::messung::ZungenTon m = lumi::messung::messeErsteZunge(s, ton(note, e), 38400);
            ++gesamt;
            const double c = m.schwingt ? 1200.0 * std::log2(m.spielHz / hzVon(note)) : 1e9;
            if (std::abs(c) < 10.0) ++getroffen;
            if (e == 0.0) einPp = m.einschwingS;
            if (e == 1.0) einFf = m.einschwingS;
        }
        CAPTURE(note);
        CHECK(einFf <= einPp + 0.02);
    }
    MESSAGE("Akkordeon: " << getroffen << " von " << gesamt << " Pruefpunkten auf ±10 Cent");
    CHECK(getroffen * 10 >= gesamt * 9);
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

TEST_CASE("InstrumentModels 34j-2 IC test_zunge: Bend nur abwaerts und gekappt, Bass-Seite, Balgwechsel")
{
    const ZungenSpiel& s = akkordeon();
    std::vector<float> t0, tu;
    const lumi::messung::ZungenTon m0 = lumi::messung::messeErsteZunge(s, ton(64, 0.6), 38400);
    rechneZungenTon(s, ton(64, 0.6), t0, 38400);
    ZungenAnschlag b = ton(64, 0.6);
    b.pitchCent = -80.0;
    const lumi::messung::ZungenTon mb = lumi::messung::messeErsteZunge(s, b, 38400);
    ZungenAnschlag u = ton(64, 0.6);
    u.pitchCent = 100.0;
    rechneZungenTon(s, u, tu, 38400);
    MESSAGE("Akkordeon E4: Bend -80 Cent ergibt " << 1200.0 * std::log2(mb.spielHz / m0.spielHz) << " Cent");
    REQUIRE(mb.schwingt);
    CHECK(mb.spielHz < m0.spielHz * std::exp2(-2.0 / 1200.0));
    CHECK(tu == t0);
    std::vector<float> bass;
    rechneZungenTon(s, ton(48, 0.6), bass, 38400);
    CHECK(rms(bass) > 0.0);
    CHECK(band(bass, 14400, 16384, hzVon(48)) > 0.0);
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
        const ZungenSpiel s = spielWieIc(name);
        const int n = (s.noteMin + s.noteMax) / 2;
        const lumi::messung::ZungenTon m = lumi::messung::messeErsteZunge(s, ton(n, 0.5), 38400);
        REQUIRE(m.schwingt);
        CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(n))) < 10.0);
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
    CHECK(abdruck(a) == icBeleg().abdruck.at("zunge_Akkordeon_m69_mf"));
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

namespace
{

/// Die Kurven des Belegs fuer ein Instrument (IC-Name) gegen eine Stimmung, Punkt fuer Punkt Bit fuer Bit.
void pruefeStimmung(const std::string& icName, const lumi::messung::ZungenStimmung& st)
{
    CAPTURE(icName);
    const auto& k = icBeleg().kurven.at(icName);
    CHECK(st.eigenfrequenz.punkte == k.at("eigenfrequenz"));
    CHECK(st.druckUnten.punkte == k.at("druckUnten"));
    CHECK(st.druckOben.punkte == k.at("druckOben"));
}

}  // namespace

TEST_CASE("InstrumentModels 34j-3 Stimmer freeReed: Melodica Bit fuer Bit wie IC, Bezug, Waechter fuer veraltete Werte")
{
    const std::vector<std::string> drei = {std::string(kEigenfrequenz), std::string(kDruckUnten), std::string(kDruckOben)};
    REQUIRE(lumi::messung::stimmerFuer(lumi::messung::stimmer(), kKernName) != nullptr);
    Werte w = werteAus(vorlage("melodica").spiel);
    const auto beginn = std::chrono::steady_clock::now();
    const lumi::messung::StimmBericht b = lumi::messung::stimmeUndNormiere(kerne(), lumi::messung::stimmer(), kKernName, w, true, true);
    const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
    MESSAGE("Melodica gestimmt und normiert in " << dauer << " s auf " << poolThreads() << " Threads (32 Tasten)");
    CHECK_MESSAGE(b.fehler.empty(), (b.fehler.empty() ? std::string() : b.fehler.front()));
    CHECK(b.gestimmt == drei);
    REQUIRE(b.mitBezug);
    CHECK(hexVon(b.bezug) == icBeleg().bezugLv.at("Melodica"));
    pruefeStimmung("Melodica", lumi::messung::ZungenStimmung{w.kurve(kEigenfrequenz), w.kurve(kDruckUnten), w.kurve(kDruckOben)});
    CHECK(richteWerte(kernTyp().tafel, w).empty());
    // der Waechter (IK2-11 a): frisch gestimmt nichts veraltet, der Bezug passt
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, w).empty());
    CHECK_FALSE(lumi::messung::bezugVeraltet(kerne(), kKernName, w));
    // absichtlich veraltet: eine Eingabe der Zunge geaendert — alle drei gestimmten Werte melden sich
    Werte alt = w;
    alt.setze("reedDamping", alt.zahl("reedDamping") * 1.5);
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, alt) == drei);
    // ein fremder Abdruck an einem gestimmten Wert genuegt
    Werte fremd = w;
    fremd.merkeAbdruck(std::string(kDruckOben), "0000");
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, fremd) == std::vector<std::string>{std::string(kDruckOben)});
    // ein falscher Bezug faellt auf
    Werte bezug = w;
    bezug.setze(std::string(kBezug), b.bezug * 1.01);
    CHECK(lumi::messung::bezugVeraltet(kerne(), kKernName, bezug));
}

TEST_CASE("InstrumentModels 34j-3 Stimmer freeReed: alle vier Bit fuer Bit wie IC Stimmung.inc (LUMIVIZ_34J_STIMMEN=1)")
{
    const char* e = std::getenv("LUMIVIZ_34J_STIMMEN");
    if (e == nullptr || std::string(e) != "1")
    {
        MESSAGE("LUMIVIZ_34J_STIMMEN=1 stimmt alle vier Instrumente am Modell (Melodica laeuft immer, im Fall davor)");
        return;
    }
    double gesamt = 0.0;
    for (const Vorlage& v : vorlagen())
    {
        const auto beginn = std::chrono::steady_clock::now();
        const lumi::messung::ZungenStimmung st = lumi::messung::zungenStimmung(v.spiel);
        const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
        gesamt += dauer;
        MESSAGE(v.name << ": " << (v.spiel.noteMax - v.spiel.noteMin + 1) << " Tasten in " << dauer << " s");
        pruefeStimmung(v.spiel.name, st);
    }
    MESSAGE("alle vier in " << gesamt << " s auf " << poolThreads() << " Threads (IC: 101 s auf vier)");
}
