/**
 ****************************************************************************************
 * @file   test_InstrumentModelsMembran.cpp
 * @brief  InstrumentModels, Stufe 34h: Klasse 5 (Membran) — der geteilte Baustein Membran und der Kern `membrane`
 *         (docs/audio/Instrumente_IC_Konzept.md 34h, TM1 a, TM2 a, TM4 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine.
 *
 * - Membran (34h-1): der Fall „ohne Luft die Bessel-Verhaeltnisse“ aus IC `rechnung/klassen/05_membran/test_trommel.cpp`
 *   (`bb7e6d6`) mit den IC-Grenzen, dazu die Nullstellen von J_m und die Form an der Stelle.
 * - Kern `membrane` (34h-3): Tafel, Fabrik, Werte hin und zurueck; der Bezug nach der Tafel gleich IC Bit fuer Bit; die
 *   Abdruecke aus `werte/trommel_ic.txt` (IC-Quellen, `docs/modellierung/anbindung-ic/abdruck_trommel.cpp`) ueber den Kern
 *   bei Stufe 0 und ueber den direkten Aufruf mit `staerke` 0,5; die IC-Faelle aus `test_trommel.cpp` mit den IC-Grenzen;
 *   der Adapter (Stufe, Spuren, Nachklang).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Pool.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/gemeinsam/Membran.hpp"
#include "InstrumentModels/trommel/TrommelKern.hpp"

#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstring>
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
using namespace lumi::kern::trommel;

namespace
{

constexpr double kPi = std::numbers::pi;

double modeHz(const std::vector<FellMode>& m, int mm, int n, int welche = 0)
{
    int k = 0;
    for (const FellMode& x : m)
        if (x.m == mm && x.n == n && k++ == welche) return x.hz;
    return 0.0;
}

const FellMode* mode(const std::vector<FellMode>& m, int mm, int n)
{
    for (const FellMode& x : m)
        if (x.m == mm && x.n == n) return &x;
    return nullptr;
}

double rms(std::span<const float> t)
{
    double s = 0.0;
    for (const float x : t) s += static_cast<double>(x) * x;
    return t.empty() ? 0.0 : std::sqrt(s / static_cast<double>(t.size()));
}

double dB(double x) { return 20.0 * std::log10(std::max(x, 1e-30)); }

double spitze(std::span<const float> t)
{
    double s = 0.0;
    for (const float x : t) s = std::max(s, std::abs(static_cast<double>(x)));
    return s;
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < t.size(); ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * double(t[ab + i]) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

std::string hexVon(double x)
{
    std::uint64_t b = 0;
    std::memcpy(&b, &x, sizeof b);
    char t[32];
    std::snprintf(t, sizeof t, "%016llx", static_cast<unsigned long long>(b));
    return t;
}

/// Die Bezuege aus IC (`werte/trommel_ic.txt`): Abdruecke der Toene und die Bezuege als Bits.
struct IcBezug
{
    std::map<std::string, std::string> abdruck;
    std::map<std::string, std::string> bezug;
};

const IcBezug& icBezug()
{
    static const IcBezug b = [] {
        IcBezug r;
        const std::filesystem::path pfad = std::filesystem::path(__FILE__).parent_path() / "werte" / "trommel_ic.txt";
        std::ifstream f(pfad);
        std::string zeile;
        while (std::getline(f, zeile))
        {
            std::istringstream z(zeile);
            std::string art, name, wert;
            z >> art >> name >> wert;
            if (art == "abdruck") r.abdruck[name] = wert;
            if (art == "bezug") r.bezug[name] = wert;
        }
        return r;
    }();
    return b;
}

/// Der IC-Name einer Vorlage (TM4 a: die Dateinamen sind englisch, die IC-Namen deutsch).
std::string icName(const Vorlage& v) { return v.spiel.name; }

const Vorlage& vorlage(std::string_view name)
{
    for (const Vorlage& v : vorlagen())
        if (v.name == name) return v;
    FAIL("keine Vorlage " << name);
    return vorlagen().front();
}

/// Die Werte einer Vorlage mit Bezug — wie `InstrumentTuneTool --reference` (einmal je Lauf).
const Werte& normierteWerte(std::string_view name)
{
    static std::mutex sperre;
    static std::map<std::string, Werte, std::less<>> fertig;
    const std::lock_guard<std::mutex> g(sperre);
    if (const auto it = fertig.find(name); it != fertig.end()) return it->second;
    Werte w = werteAus(vorlage(name).spiel);
    std::vector<std::string> fehler;
    const double bezug = normiereNachTafel(kerne(), kKernName, w, &fehler);
    REQUIRE_MESSAGE(fehler.empty(), name << ": " << (fehler.empty() ? std::string() : fehler.front()));
    w.setze(std::string(kBezug), bezug);
    const std::vector<std::string> f = richteWerte(kernTyp().tafel, w);
    REQUIRE_MESSAGE(f.empty(), name << ": " << (f.empty() ? std::string() : f.front()));
    return fertig.emplace(std::string(name), std::move(w)).first->second;
}

/// Das Spiel einer Vorlage, normiert und bereitet; `tempoPp` aus der Vorgabe (TM1 a: nicht in der Tafel — der Weg ueber
/// `staerke` der IC-Faelle braucht ihn).
TrommelSpiel bereitet(std::string_view name)
{
    TrommelSpiel s = spielAus(normierteWerte(name));
    s.tempoPp = vorlage(name).spiel.tempoPp;
    s.name = vorlage(name).spiel.name;
    bereite(s);
    return s;
}

const TrommelSpiel& pauke()
{
    static const TrommelSpiel p = bereitet("timpani");
    return p;
}

/// Ein Ton ueber den Kern (Rechenweg Note, beginn 0).
std::vector<float> kernTon(std::string_view name, int note, int stufe, Frames halten, Frames frames, std::vector<Spur> spuren = {})
{
    std::vector<std::string> fehler;
    const std::shared_ptr<const Tonkern> k = kerne().erzeuge(kKernName, normierteWerte(name), &fehler);
    REQUIRE_MESSAGE(k != nullptr, (fehler.empty() ? std::string() : fehler.front()));
    Tonauftrag a = einzelnote(note, 440.0 * std::exp2((note - 69) / 12.0), stufe, halten, frames, 48000.0);
    a.spuren = std::move(spuren);
    std::vector<float> ton;
    k->rechne(a, ton);
    return ton;
}

Spur spurAm(std::string_view name, double wert) { return Spur{std::string(name), {SpurPunkt{0, wert}}}; }

}  // namespace

TEST_CASE("InstrumentModels 34h-1 Membran: ohne Luft die Bessel-Verhaeltnisse des idealen Fells (IC test_trommel)")
{
    Fell f;
    f.abstrahlung = false;
    f.innenLuft = 0.0;
    const std::vector<FellMode> m = fellModen({f}, Kessel{}, 700.0);
    const double f01 = modeHz(m, 0, 1);
    REQUIRE(f01 > 0.0);
    CHECK(f01 == doctest::Approx(2.404826 / (2.0 * kPi * f.radiusM) * std::sqrt(f.spannungNm / f.dichteKgM2)).epsilon(1e-4));
    CHECK(modeHz(m, 1, 1) / f01 == doctest::Approx(1.5933).epsilon(1e-3));
    CHECK(modeHz(m, 2, 1) / f01 == doctest::Approx(2.1355).epsilon(1e-3));
    CHECK(modeHz(m, 0, 2) / f01 == doctest::Approx(2.2954).epsilon(1e-3));
}

TEST_CASE("InstrumentModels 34h-1 Membran: Nullstellen von J_m und die Form der Mode")
{
    // Tabellenwerte (Abramowitz & Stegun Tab. 9.5): j_01, j_02, j_11, j_21
    CHECK(besselNull(0, 1) == doctest::Approx(2.404825557695773).epsilon(1e-12));
    CHECK(besselNull(0, 2) == doctest::Approx(5.520078110286311).epsilon(1e-12));
    CHECK(besselNull(1, 1) == doctest::Approx(3.831705970207512).epsilon(1e-12));
    CHECK(besselNull(2, 1) == doctest::Approx(5.135622301840683).epsilon(1e-12));
    Fell f;
    f.abstrahlung = false;
    const std::vector<FellMode> m = fellModen({f}, Kessel{}, 700.0);
    for (const FellMode& x : m)
    {
        CAPTURE(x.m);
        CAPTURE(x.n);
        // am Rand still; m > 0 in der Mitte still; die Form ist auf den groessten Betrag 1 normiert
        CHECK(std::abs(formAn(x, 1.0)) < 1e-9);
        if (x.m > 0) CHECK(std::abs(formAn(x, 0.0)) < 1e-12);
        double groesst = 0.0;
        for (int r = 0; r <= 200; ++r) groesst = std::max(groesst, std::abs(formAn(x, r / 200.0)));
        CHECK(groesst == doctest::Approx(1.0).epsilon(1e-9));
        CHECK(x.t60S > 0.0);
        CHECK(x.masseKg > 0.0);
    }
}

TEST_CASE("InstrumentModels 34h-3 Kern membrane: Tafel, Fabrik, Vorlagen, Werte hin und zurueck")
{
    const KernTyp& t = kernTyp();
    const std::vector<std::string> fehler = pruefeTafel(t.tafel);
    CHECK_MESSAGE(fehler.empty(), (fehler.empty() ? std::string() : fehler.front()));
    REQUIRE(kerne().typ(kKernName) != nullptr);
    CHECK(t.tafel.bezugWert == kBezug);
    CHECK(t.tafel.bezugRunden == 2);
    CHECK(t.tafel.umfangVon == "noteMin");
    CHECK(t.auskunft.rechenweg == Rechenweg::Note);
    CHECK_FALSE(t.auskunft.koerperAnDerSchicht);
    // keine gestimmten Werte, keine Kurven (ein Ton fuer das ganze Instrument)
    for (const ParameterAngabe& a : t.tafel.werte)
    {
        CAPTURE(a.name);
        CHECK_FALSE(a.gestimmt);
        CHECK(a.art != WertArt::Kurve);
        CHECK_FALSE(a.quellen.empty());
    }
    // TM4 a: die sieben Vorlagen in der Reihenfolge der IC-Vorgaben
    std::vector<std::string> namen;
    for (const Vorlage& v : vorlagen()) namen.push_back(v.name);
    CHECK(namen == std::vector<std::string>{"low_tom", "timpani", "bass_drum", "snare_drum", "tabla", "conga", "bongo"});
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        Werte w = werteAus(v.spiel);
        const Werte vorher = w;
        const std::vector<std::string> f = richteWerte(t.tafel, w);
        CHECK_MESSAGE(f.empty(), (f.empty() ? std::string() : f.front()));
        // die Vorgaben liegen in den Grenzen: Richten aendert nichts, und zurueck ins Spiel und wieder in Werte gleich
        CHECK(w == vorher);
        CHECK(werteAus(spielAus(w)) == w);
        CHECK(ungemessen(t.tafel, w));
        const TrommelSpiel s = spielAus(w);
        CHECK(s.felle.size() == v.spiel.felle.size());
        CHECK(s.stimmbar == v.spiel.stimmbar);
        CHECK(static_cast<bool>(s.felle.front().belag) == static_cast<bool>(v.spiel.felle.front().belag));
    }
}

TEST_CASE("InstrumentModels 34h-3 Kern membrane: der Bezug nach der Tafel gleich IC normiere Bit fuer Bit")
{
    REQUIRE(icBezug().bezug.size() == 7);
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const double b = std::get<double>(*normierteWerte(v.name).finde(kBezug));
        CHECK(hexVon(b) == icBezug().bezug.at(icName(v)));
    }
}

TEST_CASE("InstrumentModels 34h-3 Kern membrane: Abdruecke wie IC — ueber den Kern bei Stufe 0 und direkt bei mf")
{
    // ff an der Mitte des Umfangs ueber den Kern (TM1 a: Stufe 0 = tempoFf)
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const int note = (v.spiel.note + v.spiel.noteMax) / 2;
        CHECK(abdruck(kernTon(v.name, note, 0, 12000, 24000)) == icBezug().abdruck.at("trommel_" + icName(v) + "_ff"));
    }
    // die Spielweisen der Snare ueber die Spuren (TM2 a)
    const struct
    {
        const char* ic;
        double stroke;
    } schlaege[] = {{"mitte", 1.0}, {"rand", 2.0}, {"randschlag", 3.0}, {"reifen", 4.0}, {"gedaempft", 5.0}};
    for (const auto& x : schlaege)
    {
        CAPTURE(x.ic);
        CHECK(abdruck(kernTon("snare_drum", 38, 0, 12000, 24000, {spurAm(kSpurSchlag, x.stroke)})) ==
              icBezug().abdruck.at(std::string("trommel_Snare_") + x.ic + "_ff"));
    }
    CHECK(abdruck(kernTon("snare_drum", 38, 0, 12000, 24000, {spurAm(kSpurOhneTeppich, 1.0)})) == icBezug().abdruck.at("trommel_Snare_ohneTeppich_ff"));
    // der Wirbel ueber die Laenge der Note (0,8 s)
    CHECK(abdruck(kernTon("snare_drum", 38, 0, 38400, 48000, {spurAm(kSpurWirbel, 1.0)})) == icBezug().abdruck.at("trommel_Snare_wirbel_ff"));
    // die zwei IC-Abdruecke (mf, staerke 0,5) ueber den direkten Aufruf
    std::vector<float> t;
    rechneTrommelTon(bereitet("low_tom"), TrommelAnschlag{}, t, 24000);
    CHECK(abdruck(t) == icBezug().abdruck.at("trommel_Tom_mf"));
    TrommelAnschlag p;
    p.note = 52;
    rechneTrommelTon(pauke(), p, t, 24000);
    CHECK(abdruck(t) == icBezug().abdruck.at("trommel_Pauke_m52_mf"));
}

TEST_CASE("InstrumentModels 34h-3 Pauke gegen Packer 1993 Tab. 7.1 (IC test_trommel): (2,1) : (3,1) : (4,1) zu (1,1) = 1,50 : 1,97 : 2,44")
{
    const std::vector<FellMode>& m = *pauke().moden;
    const double f11 = modeHz(m, 1, 1);
    REQUIRE(f11 > 0.0);
    CHECK(modeHz(m, 2, 1) / f11 == doctest::Approx(1.50).epsilon(0.04));
    CHECK(modeHz(m, 3, 1) / f11 == doctest::Approx(1.97).epsilon(0.04));
    CHECK(modeHz(m, 4, 1) / f11 == doctest::Approx(2.44).epsilon(0.04));
    CHECK(modeHz(m, 0, 1) / f11 == doctest::Approx(0.81).epsilon(0.08));
    const FellMode* a = mode(m, 0, 1);
    const FellMode* b = mode(m, 1, 1);
    const FellMode* c = mode(m, 2, 1);
    REQUIRE((a && b && c));
    CHECK(a->t60S < 0.3);
    CHECK(b->t60S > 0.3);
    CHECK(b->t60S < 1.5);
    CHECK(c->t60S > b->t60S);
}

TEST_CASE("InstrumentModels 34h-3 zwei Felle und Belag (IC test_trommel): das (0,1)-Paar des Toms, die Tabla harmonisch")
{
    const TrommelSpiel tom = bereitet("low_tom");
    const double a = modeHz(*tom.moden, 0, 1, 0), b = modeHz(*tom.moden, 0, 1, 1);
    REQUIRE(b > 0.0);
    CHECK(b / a > 1.3);
    const TrommelSpiel t = bereitet("tabla");
    const std::vector<FellMode>& m = *t.moden;
    const double f11 = modeHz(m, 1, 1);
    CHECK(modeHz(m, 2, 1) / f11 == doctest::Approx(1.5).epsilon(0.03));
    CHECK(modeHz(m, 0, 2) / f11 == doctest::Approx(1.5).epsilon(0.03));
    CHECK(modeHz(m, 3, 1) / f11 == doctest::Approx(2.0).epsilon(0.03));
    CHECK(modeHz(m, 1, 2) / f11 == doctest::Approx(2.0).epsilon(0.03));
}

TEST_CASE("InstrumentModels 34h-3 Trommel (IC test_trommel): Kontakt und Kraft, lauter mit der Staerke, Gleiten")
{
    const TrommelSpiel tom = bereitet("low_tom");
    double vorher = -1e9;
    for (const double e : {0.0, 0.5, 1.0})
    {
        TrommelAnschlag a;
        a.staerke = e;
        TrommelMessung m;
        std::vector<float> t;
        rechneTrommelTon(tom, a, t, 48000, &m);
        CAPTURE(e);
        CHECK(m.kontaktS < 0.015);
        CHECK(m.kraftSpitzeN > 3.0);
        CHECK(m.kraftSpitzeN < 300.0);
        CHECK(dB(rms(t)) > vorher);
        vorher = dB(rms(t));
    }
    TrommelAnschlag ff;
    ff.staerke = 1.0;
    TrommelMessung ms, mp;
    std::vector<float> t;
    rechneTrommelTon(bereitet("snare_drum"), ff, t, 48000, &ms);
    ff.note = 52;
    rechneTrommelTon(pauke(), ff, t, 48000, &mp);
    CHECK(ms.gleitenCent > 2.0);
    CHECK(ms.gleitenCent < 100.0);
    CHECK(mp.gleitenCent < ms.gleitenCent);
}

TEST_CASE("InstrumentModels 34h-3 Pauke (IC test_trommel): die Note waehlt die Spannung, Glissando ueber das Pedal und die Spur pitch")
{
    for (const int note : {48, 52, 55})
    {
        TrommelAnschlag a;
        a.note = note;
        std::vector<float> t;
        rechneTrommelTon(pauke(), a, t, 48000);
        const double f = 440.0 * std::exp2((note - 69) / 12.0);
        CAPTURE(note);
        CHECK(band(t, 2400, 16384, f) > 3.0 * band(t, 2400, 16384, f * std::exp2(1.0 / 12.0)));
    }
    TrommelAnschlag g;
    g.note = 48;
    g.glissandoCent = 400.0;
    std::vector<float> a, b;
    rechneTrommelTon(pauke(), g, a, 48000);
    g.glissandoCent = 0.0;
    rechneTrommelTon(pauke(), g, b, 48000);
    CHECK(a != b);
    // 34h-3: die Spur pitch mit zwei Punkten ist die IC-Rampe (dieselben Toene bis auf die Rundung der Rampe), eine Spur
    // bei 0 Cent laesst den Ton, wie er ist
    const std::vector<float> ohne = kernTon("timpani", 48, -6, 12000, 48000);
    const std::vector<float> null = kernTon("timpani", 48, -6, 12000, 48000, {spurAm(spur::kPitch, 0.0)});
    CHECK(ohne == null);
    Spur rampe{std::string(spur::kPitch), {SpurPunkt{0, 0.0}, SpurPunkt{48000, 400.0}}};
    const std::vector<float> mit = kernTon("timpani", 48, -6, 12000, 48000, {rampe});
    CHECK(mit != ohne);
    // am Ende 400 Cent hoeher: die (1,1) liegt eine grosse Terz hoeher (E3 statt C3) — staerker als auf C3
    const double c3 = 440.0 * std::exp2((48 - 69) / 12.0), e3 = c3 * std::exp2(4.0 / 12.0);
    CHECK(band(mit, 36000, 8192, e3) > band(mit, 36000, 8192, c3));
}

TEST_CASE("InstrumentModels 34h-3 Trommel (IC test_trommel): Spielweisen, Wirbel, Teppich, alle Instrumente klingen")
{
    const TrommelSpiel snare = bereitet("snare_drum");
    std::vector<float> normal;
    rechneTrommelTon(snare, TrommelAnschlag{}, normal, 24000);
    for (const Schlag s : {Schlag::Mitte, Schlag::Rand, Schlag::Randschlag, Schlag::Reifen, Schlag::Gedaempft})
    {
        TrommelAnschlag a;
        a.schlag = s;
        std::vector<float> t;
        rechneTrommelTon(snare, a, t, 24000);
        CHECK(t != normal);
        CHECK(rms(t) > 0.0);
    }
    TrommelAnschlag w;
    w.wirbelS = 0.8;
    std::vector<float> wirbel;
    rechneTrommelTon(snare, w, wirbel, 48000);
    double spaet = 0.0;
    for (std::size_t i = 28800; i < 33600; ++i) spaet += double(wirbel[i]) * wirbel[i];
    CHECK(spaet > 0.0);
    TrommelAnschlag o;
    o.ohneTeppich = true;
    std::vector<float> ohne;
    rechneTrommelTon(snare, o, ohne, 24000);
    double em = 0.0, eo = 0.0;
    for (double f = 3000.0; f <= 8000.0; f += 100.0)
    {
        em += std::pow(band(normal, 0, 8192, f), 2);
        eo += std::pow(band(ohne, 0, 8192, f), 2);
    }
    CHECK(em > eo);
    for (const char* name : {"bass_drum", "conga", "bongo"})
    {
        CAPTURE(name);
        std::vector<float> t;
        rechneTrommelTon(bereitet(name), TrommelAnschlag{}, t, 24000);
        CHECK(rms(t) > 0.0);
    }
}

TEST_CASE("InstrumentModels 34h-3 Trommel: Determinismus auf Threads")
{
    const TrommelSpiel tom = bereitet("low_tom");
    std::vector<float> a;
    rechneTrommelTon(tom, TrommelAnschlag{}, a, 24000);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneTrommelTon(tom, TrommelAnschlag{}, q[i], 24000); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
}

TEST_CASE("InstrumentModels 34h-3 Kern membrane: der Adapter — Stufe in dB (TM1 a), Spuren, Nachklang")
{
    // TM1 a: die Geschwindigkeit folgt der Stufe in dB; der Ton wird um rund so viel leiser (S179 gemessen: Tom −20,3 dB)
    const std::vector<float> ff = kernTon("low_tom", 45, 0, 12000, 48000);
    const std::vector<float> leise = kernTon("low_tom", 45, -20, 12000, 48000);
    const double abstand = dB(rms(leise) / rms(ff));
    MESSAGE("Tom Stufe -20 gegen 0: " << abstand << " dB");
    CHECK(abstand < -17.0);
    CHECK(abstand > -24.0);
    CHECK(spitze(ff) == doctest::Approx(kNormSpitze).epsilon(0.05));
    // die Spur stroke rundet und klemmt
    CHECK(schlagAus(-1.0) == Schlag::Normal);
    CHECK(schlagAus(3.4) == Schlag::Randschlag);
    CHECK(schlagAus(9.0) == Schlag::Gedaempft);
    // ohne Wirbel ist die Laenge der Note gleich (kein Daempfer): derselbe Ton bei kurzer und langer Note
    CHECK(kernTon("low_tom", 45, -6, 1200, 24000) == kernTon("low_tom", 45, -6, 24000, 24000));
    // die feste Trommel spielt auf jeder Note denselben Ton (die Rolle im Kit, Note 36)
    CHECK(kernTon("low_tom", 36, -6, 1200, 24000) == kernTon("low_tom", 45, -6, 1200, 24000));
    // der Nachklang: danach ist der Ton um mindestens 50 dB unter seiner Spitze
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const std::shared_ptr<const Tonkern> k = kerne().erzeuge(kKernName, normierteWerte(v.name));
        REQUIRE(k != nullptr);
        const int note = (v.spiel.note + v.spiel.noteMax) / 2;
        Tonauftrag a = einzelnote(note, 440.0 * std::exp2((note - 69) / 12.0), 0, 12000, 0, 48000.0);
        const Frames rest = k->nachklang(a, 48000 * 20);
        CHECK(rest > 4800);
        CHECK(rest < 48000 * 20);
        a.frames = 12000 + rest;
        std::vector<float> ton;
        k->rechne(a, ton);
        const std::span<const float> schluss = std::span<const float>(ton).subspan(ton.size() - 4800, 4800);
        CHECK(dB(rms(schluss) / spitze(ton)) < -50.0);
    }
    // innere Groessen
    const std::shared_ptr<const Tonkern> k = kerne().erzeuge(kKernName, normierteWerte("snare_drum"));
    Innere innere;
    innere.gewuenscht = {"contact", "glide"};
    std::vector<float> ton;
    k->rechne(einzelnote(38, 73.4, 0, 12000, 48000, 48000.0), ton, &innere);
    REQUIRE(innere.ergebnis.size() == 2);
    CHECK(innere.ergebnis[0].werte[0] > 0.0);
    CHECK(innere.ergebnis[1].werte[0] > 0.0);
}
