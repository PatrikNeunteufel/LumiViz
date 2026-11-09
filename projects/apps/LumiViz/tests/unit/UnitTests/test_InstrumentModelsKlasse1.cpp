/**
 ****************************************************************************************
 * @file   test_InstrumentModelsKlasse1.cpp
 * @brief  InstrumentModels, Stufe 34i: Klasse 1 ohne Fluegel — die Vorgaben auf dem Kern `hammerString` (Upright,
 *         Honky-tonk, CP-70) und der Kern `struckString` (Clavinet; Hackbrett, Cimbalom nicht ausgeliefert)
 *         (docs/audio/Instrumente_IC_Konzept.md 34i, KL1 a–KL4 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine. Bezug: `werte/klasse1_ic.txt` (IC `rechnung/werte/abdruck_msvc.txt`,
 * `bb7e6d6`).
 *
 * - 34i-1 Beleg: die Vorgabe des Fluegels rechnet wie IC `test_hammer_ton.cpp` die fuenf Abdruecke
 *   `hammer_vorgabe_m*_a-12_d500` (Stufe −12, 0,5 s gehalten, Nachklang bis 4 s). Nur wenn sie Bit fuer Bit stimmen,
 *   sind die IC-Vorgaben von Upright, Honky-tonk und CP-70 auf diesen Kern uebertragbar.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Fft.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentCore/Tonauftrag.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/hammer/HammerKern.hpp"
#include "InstrumentModels/hammer/HammerSaite.hpp"
#include "InstrumentModels/hammer/HammerTon.hpp"
#include "InstrumentModels/saite/SaiteKern.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <functional>
#include <memory>
#include <numbers>
#include <utility>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::kern;
using namespace lumi::kern::hammer;
namespace saite = lumi::kern::saite;

namespace
{

/// Die erste Taste ohne Daempfer in IC (`HammerSaite.hpp:239`, `kUngedaempftAbVorgabe`); in LumiViz `undampedFrom`.
constexpr int kIcUngedaempftAb = 89;

/// Die Abdruecke aus IC (`werte/klasse1_ic.txt`).
const std::map<std::string, std::string>& icAbdruecke()
{
    static const std::map<std::string, std::string> m = [] {
        std::map<std::string, std::string> r;
        const std::filesystem::path pfad = std::filesystem::path(__FILE__).parent_path() / "werte" / "klasse1_ic.txt";
        std::ifstream f(pfad);
        std::string zeile;
        while (std::getline(f, zeile))
        {
            std::istringstream z(zeile);
            std::string art, name;
            z >> art >> name;
            if (art == "abdruck") z >> r[name];
        }
        return r;
    }();
    return m;
}

/// Der Anschlag wie IC `test_hammer_ton.cpp` (`anschlag`) und `test_hammer_rest.cpp` (`hammerAnschlag`).
HammerAnschlag icAnschlag(int taste, int stufe, double haltenS)
{
    HammerAnschlag a;
    a.taste = taste;
    a.hz = 440.0 * std::exp2((taste - 69) / 12.0);
    a.stufe = stufe;
    a.rate = 48000;
    a.halten = std::llround(haltenS * a.rate);
    a.gedaempft = taste < kIcUngedaempftAb;
    return a;
}

double rms(const std::vector<float>& t)
{
    double s = 0.0;
    for (const float x : t) s += static_cast<double>(x) * x;
    return t.empty() ? 0.0 : std::sqrt(s / static_cast<double>(t.size()));
}

/// Der spektrale Schwerpunkt der ersten `n` Werte in Hz (wie IC `test_hammer_rest.cpp`).
double schwerpunkt(const std::vector<float>& t, std::size_t n)
{
    std::vector<double> x(n, 0.0);
    for (std::size_t i = 0; i < n && i < t.size(); ++i) x[i] = t[i];
    std::vector<std::complex<double>> sp(n / 2 + 1);
    fft().vorwaerts(x, sp);
    double a = 0.0, b = 0.0;
    for (std::size_t k = 1; k < sp.size(); ++k)
    {
        a += static_cast<double>(k) * std::norm(sp[k]);
        b += std::norm(sp[k]);
    }
    return a / b * 48000.0 / static_cast<double>(n);
}

double rmsAb(const std::vector<float>& t, std::size_t ab, std::size_t n)
{
    double s = 0.0;
    for (std::size_t i = ab; i < ab + n && i < t.size(); ++i) s += static_cast<double>(t[i]) * t[i];
    return std::sqrt(s / static_cast<double>(n));
}

/// Die staerkste Frequenz zwischen `von` und `bis` in einem Ausschnitt (Hann, Nullen auf 2^18; wie IC).
double staerksteHz(const std::vector<float>& x, std::size_t ab, std::size_t n, double von, double bis)
{
    const std::size_t lang = 1u << 18;
    std::vector<double> z(lang, 0.0);
    for (std::size_t i = 0; i < n && ab + i < x.size(); ++i)
        z[i] = x[ab + i] * (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(n)));
    std::vector<std::complex<double>> s(lang / 2 + 1);
    fft().vorwaerts(z, s);
    std::size_t beste = 0;
    for (std::size_t k = 1; k < s.size(); ++k)
    {
        const double f = static_cast<double>(k) * 48000.0 / static_cast<double>(lang);
        if (f < von || f > bis) continue;
        if (beste == 0 || std::abs(s[k]) > std::abs(s[beste])) beste = k;
    }
    return static_cast<double>(beste) * 48000.0 / static_cast<double>(lang);
}

/// Der Anschlag wie IC `test_hammer_rest.cpp` (`saitenAnschlag`).
saite::SaitenAnschlag icSaitenAnschlag(int note, int stufe, double haltenS = 0.5)
{
    saite::SaitenAnschlag a;
    a.note = note;
    a.hz = 440.0 * std::exp2((note - 69) / 12.0);
    a.stufe = stufe;
    a.halten = std::llround(haltenS * a.rate);
    return a;
}

/// Der Bezug wie IC `SaitenVorgaben.cpp` `normiere`: Spitze 0,5 an der mittleren Taste, Stufe 0, 0,5 s gehalten und
/// gerechnet.
double icBezug(saite::SaitenSpiel s)
{
    s.ausgangBezug = 1.0;
    saite::SaitenAnschlag a;
    a.note = (s.noteMin + s.noteMax) / 2;
    a.hz = 440.0 * std::exp2((a.note - 69) / 12.0);
    a.stufe = 0;
    a.halten = a.rate / 2;
    std::vector<float> ton;
    saite::rechneSaitenTon(s, a, ton, a.rate / 2);
    double sp = 0.0;
    for (const float x : ton) sp = std::max(sp, static_cast<double>(std::abs(x)));
    return sp > 0.0 ? sp / 0.5 : 1.0;
}

/// Das Spiel einer Vorlage mit dem Bezug wie in IC (fuer die Abdruecke).
saite::SaitenSpiel mitIcBezug(const saite::Vorlage& v)
{
    saite::SaitenSpiel s = v.spiel;
    s.ausgangBezug = icBezug(s);
    return s;
}

const saite::Vorlage& saitenVorlage(std::string_view name)
{
    for (const saite::Vorlage& v : saite::vorlagen())
        if (v.name == name) return v;
    FAIL("keine Vorlage " << name);
    return saite::vorlagen().front();
}

}  // namespace

TEST_CASE("InstrumentModels 34i-3 Kern struckString: Tafel, Fabrik, Vorlagen, Werte hin und zurueck")
{
    const std::vector<std::string> namen = kerne().namen();
    CHECK(std::find(namen.begin(), namen.end(), std::string(saite::kKernName)) != namen.end());
    const KernTyp& t = saite::kernTyp();
    CHECK(pruefeTafel(t.tafel).empty());
    CHECK(t.tafel.umfangVon == "noteMin");
    CHECK(t.tafel.bezugWert == std::string(saite::kBezug));
    std::vector<std::string> mit;
    for (const saite::Vorlage& v : saite::vorlagen())
        if (v.ausgeliefert) mit.push_back(v.name);
    CHECK(saite::vorlagen().size() == 3);
    CHECK(mit == std::vector<std::string>{"clavinet_modal"});  // KL1 a: Hackbrett und Cimbalom ohne Datei
    for (const saite::Vorlage& v : saite::vorlagen())
    {
        CAPTURE(v.name);
        Werte w = saite::werteVon(v);
        CHECK(richteWerte(t.tafel, w).empty());
        CHECK(tastenumfang(t.tafel, w) == std::pair<int, int>{v.spiel.noteMin, v.spiel.noteMax});
        // hin und zurueck: Werte → Spiel → Werte
        Werte zurueck = saite::werteAus(saite::spielAus(w));
        CHECK(richteWerte(t.tafel, zurueck).empty());
        for (const auto& [n, wert] : w.felder)
            if (const Wert* z = zurueck.finde(n); z == nullptr || !(*z == wert)) CHECK_MESSAGE(false, "anders: " << n);
        // jede Kurve traegt eine Quelle (WZ1 a)
        for (const auto& [n, wert] : w.felder)
            if (const Kurve* k = std::get_if<Kurve>(&wert); k != nullptr && !k->punkte.empty()) CHECK_MESSAGE(!k->quellen.empty(), n);
        CHECK(ungemessen(t.tafel, w));
    }
}

TEST_CASE("InstrumentModels 34i-3: Clavinet, Hackbrett, Cimbalom Bit fuer Bit wie IC; der Bezug ueber die Tafel wie IC normiere")
{
    const auto& ic = icAbdruecke();
    const std::pair<const char*, const char*> namen[] = {
        {"clavinet_modal", "klasse1_Clavinet_m60_a-12"}, {"hammered_dulcimer", "klasse1_Hackbrett_m60_a-12"}, {"cimbalom", "klasse1_Cimbalom_m60_a-12"}};
    for (const auto& [name, abdruckName] : namen)
    {
        CAPTURE(name);
        const saite::Vorlage& v = saitenVorlage(name);
        const saite::SaitenSpiel s = mitIcBezug(v);
        // wie IC: Taste 60, Stufe −12, 0,5 s gehalten, 1 s gerechnet
        const saite::SaitenAnschlag a = icSaitenAnschlag(60, -12);
        std::vector<float> direkt;
        saite::rechneSaitenTon(s, a, direkt, 48000);
        CHECK(abdruck(direkt) == ic.at(abdruckName));
        // der Bezug ueber die Tafel (IK2-10 a) gleich dem aus IC
        Werte w = saite::werteVon(v);
        REQUIRE(richteWerte(saite::kernTyp().tafel, w).empty());
        std::vector<std::string> fehler;
        const double bezug = normiereNachTafel(kerne(), saite::kKernName, w, &fehler);
        CHECK(fehler.empty());
        CHECK(bezug == s.ausgangBezug);
        // ueber den Kern aus den Werten mit diesem Bezug
        w.setze(std::string(saite::kBezug), bezug);
        const std::shared_ptr<const Tonkern> kern = kerne().erzeuge(saite::kKernName, w);
        REQUIRE(kern);
        std::vector<float> ueberKern;
        kern->rechne(einzelnote(60, a.hz, -12, a.halten, 48000, 48000.0), ueberKern);
        CHECK(ueberKern == direkt);
    }
}

TEST_CASE("InstrumentModels 34i-3: Hackbrett je Clip — die anderen Choere schwingen mit, der Clip ist die Summe (IC-Abdruck)")
{
    const saite::SaitenSpiel h = mitIcBezug(saitenVorlage("hammered_dulcimer"));
    REQUIRE(h.mitschwingen);
    const saite::SaitenAnschlag a = icSaitenAnschlag(60, -6, 4.0);
    std::vector<float> je, clip;
    saite::rechneSaitenTon(h, a, je, 4 * 48000);
    saite::rechneSaitenClip(h, {{0.0, a}}, clip, 4 * 48000);
    CHECK(je != clip);
    std::vector<float> zwei, c1, c2;
    const saite::SaitenAnschlag b = icSaitenAnschlag(67, -6, 4.0);
    saite::rechneSaitenClip(h, {{0.0, a}, {0.5, b}}, zwei, 2 * 48000);
    saite::rechneSaitenClip(h, {{0.0, a}}, c1, 2 * 48000);
    saite::rechneSaitenClip(h, {{0.5, b}}, c2, 2 * 48000);
    double abw = 0.0;
    for (std::size_t i = 0; i < zwei.size(); ++i) abw = std::max(abw, std::abs(double(zwei[i]) - (double(c1[i]) + c2[i])));
    CHECK(abw < 1e-6);
    CHECK(abdruck(zwei) == icAbdruecke().at("saite_Hackbrett_clip_C4_G4"));
    // Cimbalom ohne Pedal: die Daempferleiste liegt auf allen Choeren — der Clip ist der Ton
    const saite::SaitenSpiel c = mitIcBezug(saitenVorlage("cimbalom"));
    std::vector<float> cj, cc;
    saite::rechneSaitenTon(c, a, cj, 48000);
    saite::rechneSaitenClip(c, {{0.0, a}}, cc, 48000);
    CHECK(cj == cc);
}

TEST_CASE("InstrumentModels 34i-3: Clavinet — drei Halbtoene nach dem Loslassen, Tastendruck mit Grenze 0, Schieber, Tonabnehmer (IC)")
{
    const saite::SaitenSpiel c = mitIcBezug(saitenVorlage("clavinet_modal"));
    saite::SaitenAnschlag a = icSaitenAnschlag(57, -6, 0.3);
    std::vector<float> ton;
    saite::SaitenMessung m;
    saite::rechneSaitenTon(c, a, ton, a.rate, &m);
    const double f = a.hz;
    CHECK(m.hzNachLoslassen == doctest::Approx(f * std::exp2(-3.0 / 12.0)).epsilon(2e-3));
    CHECK(staerksteHz(ton, 2400, 9600, 0.8 * f, 1.2 * f) == doctest::Approx(f).epsilon(0.005));
    CHECK(staerksteHz(ton, static_cast<std::size_t>(a.halten) + 480, 4800, 0.7 * f, 0.95 * f) == doctest::Approx(f * std::exp2(-0.25)).epsilon(0.01));
    saite::SaitenAnschlag d = a;
    d.tastendruckCent = 30.0;
    std::vector<float> gedrueckt;
    saite::rechneSaitenTon(c, d, gedrueckt, a.rate);
    CHECK(gedrueckt == ton);
    saite::SaitenAnschlag s = a;
    s.schieber = 1.0;
    s.halten = a.rate;
    a.halten = a.rate;
    std::vector<float> mit, ohne;
    saite::rechneSaitenTon(c, s, mit, a.rate);
    saite::rechneSaitenTon(c, a, ohne, a.rate);
    CHECK(rmsAb(mit, 24000, 12000) < 0.1 * rmsAb(ohne, 24000, 12000));
    std::vector<float> leise, laut;
    saite::rechneSaitenTon(c, icSaitenAnschlag(45, -20), leise, 16384);
    saite::rechneSaitenTon(c, icSaitenAnschlag(45, 0), laut, 16384);
    CHECK(schwerpunkt(laut, 16384) > schwerpunkt(leise, 16384));
    // die Wahl der Tonabnehmer: beide = Steg + Mitte, Gegenphase = Steg − Mitte
    saite::SaitenAnschlag p = icSaitenAnschlag(29, -6, 1.0);
    std::array<std::vector<float>, 4> t;
    for (int i = 0; i < 4; ++i)
    {
        p.abnehmerWahl = saite::abnehmerWahlAus(i);
        saite::rechneSaitenTon(c, p, t[static_cast<std::size_t>(i)], 48000);
    }
    double abwSumme = 0.0, abwDiff = 0.0, sp = 0.0;
    for (std::size_t i = 0; i < t[0].size(); ++i)
    {
        abwSumme = std::max(abwSumme, std::abs(double(t[0][i]) - (double(t[1][i]) + t[2][i])));
        abwDiff = std::max(abwDiff, std::abs(double(t[3][i]) - (double(t[1][i]) - t[2][i])));
        sp = std::max(sp, std::abs(double(t[0][i])));
    }
    CHECK(abwSumme < 1e-5 * sp);
    CHECK(abwDiff < 1e-5 * sp);
    CHECK(t[1] != t[2]);
    // die Spuren am Kern: Schieber, Tastendruck, Tonabnehmer am Beginn der Note
    Werte w = saite::werteVon(saitenVorlage("clavinet_modal"));
    REQUIRE(richteWerte(saite::kernTyp().tafel, w).empty());
    w.setze(std::string(saite::kBezug), c.ausgangBezug);
    const std::shared_ptr<const Tonkern> kern = kerne().erzeuge(saite::kKernName, w);
    REQUIRE(kern);
    Tonauftrag auftrag = einzelnote(29, p.hz, -6, p.halten, 48000, 48000.0);
    auftrag.spuren.push_back(Spur{std::string(saite::kSpurAbnehmer), {{0, 3.0}}});
    std::vector<float> ueberKern;
    kern->rechne(auftrag, ueberKern);
    CHECK(ueberKern == t[3]);
}

TEST_CASE("InstrumentModels 34i-1: die Vorgabe des Fluegels Bit fuer Bit wie IC (hammer_vorgabe, fuenf Tasten)")
{
    const auto& ic = icAbdruecke();
    REQUIRE_MESSAGE(ic.size() == 12, "werte/klasse1_ic.txt fehlt oder ist unvollstaendig");
    const HammerSaite& s = hammerSaiteVorgabe();
    for (const int taste : {21, 40, 60, 84, 100})
    {
        const HammerAnschlag a = icAnschlag(taste, -12, 0.5);
        std::vector<float> ton;
        rechneHammerTon(s, a, ton, a.halten + hammerNachklang(s, a, 48000 * 4));
        const std::string name = "hammer_vorgabe_m" + std::to_string(taste) + "_a-12_d500";
        CAPTURE(name);
        CHECK(abdruck(ton) == ic.at(name));
    }
}

TEST_CASE("InstrumentModels 34i-1: Upright, Honky-tonk, CP-70 Bit fuer Bit wie IC, direkt und ueber den Kern")
{
    const auto& ic = icAbdruecke();
    // die Namen in IC (`test_hammer_rest.cpp`, „Klasse 1 Rest: Determinismus, Abdruck, Rechenzeit“) zu den Vorlagen
    const std::pair<const char*, const char*> namen[] = {
        {"upright_piano", "klasse1_Upright_m60_a-12"},
        {"honky_tonk_piano", "klasse1_Honkytonk_m60_a-12"},
        {"electric_grand_piano", "klasse1_CP70_m60_a-12"},
    };
    REQUIRE(vorlagen().size() == 3);
    for (std::size_t i = 0; i < 3; ++i)
    {
        const Vorlage& v = vorlagen()[i];
        CAPTURE(v.name);
        CHECK(v.name == namen[i].first);
        // wie IC: Taste 60, Stufe −12, 0,5 s gehalten, 1 s gerechnet
        const HammerAnschlag a = icAnschlag(60, -12, 0.5);
        std::vector<float> direkt;
        rechneHammerTon(v.saite, a, direkt, 48000);
        CHECK(abdruck(direkt) == ic.at(namen[i].second));
        // ueber den Kern aus den Werten der Vorlage (gerichtet wie aus der Datei)
        Werte w = werteVon(v);
        CHECK(richteWerte(kernTyp().tafel, w).empty());
        // KL4 a: der Umfang in der Tafel (der CP-70 28–100); KL4-1 a: gerichtet bleiben die Kurven, wie sie sind
        CHECK(tastenumfang(kernTyp().tafel, w) == v.umfang);
        for (const KurvenAngabe& k : kurvenAngaben()) CHECK(w.kurve(k.name).punkte == (v.saite.*k.feld).punkte);
        const std::shared_ptr<const Tonkern> kern = kerne().erzeuge(kKernName, w);
        REQUIRE(kern);
        std::vector<float> ueberKern;
        kern->rechne(einzelnote(60, a.hz, -12, a.halten, 48000, 48000.0), ueberKern);
        CHECK(ueberKern == direkt);
        // ungemessen: jede Kurve ist hoechstens Rechnung (IK2-9)
        CHECK(ungemessen(kernTyp().tafel, w));
    }
}

TEST_CASE("InstrumentModels 34i-1: Upright — Bass gekappt auf 1,189 m bei gleicher Spannung (IC test_hammer_rest)")
{
    const HammerSaite& f = hammerSaiteVorgabe();
    const HammerSaite u = baueVorgabeUpright();
    for (const int t : {24, 30, 36})
    {
        CAPTURE(t);
        const double lf = wertAn(f.saitenLaengeM, t, Skala::Logarithmisch);
        const double lu = wertAn(u.saitenLaengeM, t, Skala::Logarithmisch);
        CHECK(lu == doctest::Approx(std::min(lf, 46.8 * 0.0254)).epsilon(1e-9));
        const double faktor = (lf / lu) * (lf / lu);
        CHECK(wertAn(u.inharmonizitaet, t, Skala::Logarithmisch) ==
              doctest::Approx(wertAn(f.inharmonizitaet, t, Skala::Logarithmisch) * faktor).epsilon(1e-9));
        // T0 = 4L²f0²µ bleibt gleich
        const double tf = lf * lf * wertAn(f.saitenMasseGm, t, Skala::Logarithmisch);
        const double tu = lu * lu * wertAn(u.saitenMasseGm, t, Skala::Logarithmisch);
        CHECK(tu == doctest::Approx(tf).epsilon(1e-9));
    }
    // die Quellen: was vom Fluegel kommt, behaelt seine; was gerechnet ist, nennt den U1 dazu
    CHECK(u.abklingS.quellen == f.abklingS.quellen);
    CHECK(std::find(u.saitenLaengeM.quellen.begin(), u.saitenLaengeM.quellen.end(), "yamaha-u1") != u.saitenLaengeM.quellen.end());
    CHECK(u.anschlagStelle.quellen == std::vector<std::string>{"lumiviz"});
}

TEST_CASE("InstrumentModels 34i-1: Upright — Practice leiser und dunkler, Soft mit Faktor 1 Bit fuer Bit wie ohne")
{
    const HammerSaite u = baueVorgabeUpright();
    const Pedalwerte k = vorlagen()[0].pedale;
    const HammerAnschlag a = icAnschlag(60, -6, 0.5);
    std::vector<float> ohne, mit, soft;
    const auto frames = a.halten + hammerNachklang(u, a, 96000);
    rechneHammerTon(u, a, ohne, frames);
    HammerAnschlag m = a;
    m.moderatorSteife = k.moderatorSteife;
    m.moderatorExponent = k.moderatorExponent;
    rechneHammerTon(u, m, mit, frames);
    CHECK(rms(mit) < rms(ohne));
    CHECK(schwerpunkt(mit, 16384) < schwerpunkt(ohne, 16384));
    HammerAnschlag s = a;
    s.tempoFaktor = k.softFaktor;
    rechneHammerTon(u, s, soft, frames);
    CHECK(soft == ohne);
}

TEST_CASE("InstrumentModels 34i-2 MESSUNG Kurven des CP-70 auf 28-100 geklemmt (LUMIVIZ_34I_KLEMMEN, meldet nur Zahlen)")
{
    // der Befund vor KL4-1 a: was das Klemmen der Stuetzpunkte auf den Umfang des Instruments am CP-70 aenderte (Journal J55)
    if (std::getenv("LUMIVIZ_34I_KLEMMEN") == nullptr)
    {
        MESSAGE("LUMIVIZ_34I_KLEMMEN nicht gesetzt — keine Messung");
        return;
    }
    const HammerSaite c = baueVorgabeCp70();
    for (const KurvenAngabe& k : kurvenAngaben())
    {
        Kurve g = c.*k.feld;
        richteKurve(g, k.von, k.bis, g, 28, 100);
        double groesste = 0.0;
        int beiTaste = 0;
        int anders = 0;
        for (int t = 28; t <= 100; ++t)
        {
            const double a = wertAn(c.*k.feld, t, k.skala);
            const double b = wertAn(g, t, k.skala);
            if (a != b) ++anders;
            const double rel = a != 0.0 ? std::abs(b - a) / std::abs(a) : std::abs(b - a);
            if (rel > groesste)
            {
                groesste = rel;
                beiTaste = t;
            }
        }
        MESSAGE(k.name << ": Tasten anders " << anders << " von 73, groesste Abweichung " << 100.0 * groesste << " % bei Taste " << beiTaste);
    }
    // der Ton: ungeklemmt (wie IC) gegen geklemmt, je Taste gleich oder der Unterschied in dB (RMS der Differenz gegen den Ton)
    HammerSaite g = c;
    for (const KurvenAngabe& k : kurvenAngaben()) richteKurve(g.*k.feld, k.von, k.bis, g.*k.feld, 28, 100);
    for (const int t : {28, 30, 33, 36, 40, 45, 50, 60, 70, 85, 93, 96, 100})
    {
        const HammerAnschlag a = icAnschlag(t, -12, 0.5);
        std::vector<float> x, y;
        rechneHammerTon(c, a, x, 48000);
        rechneHammerTon(g, a, y, 48000);
        std::vector<float> d(x.size());
        for (std::size_t i = 0; i < x.size(); ++i) d[i] = x[i] - y[i];
        MESSAGE("Taste " << t << ": " << (x == y ? std::string("Bit fuer Bit gleich") : "Differenz " + std::to_string(20.0 * std::log10(rms(d) / rms(x))) + " dB"));
    }
}

TEST_CASE("InstrumentModels 34i-1: Honky-tonk ±1 Hz bei A4, CP-70 eine Saite bis MIDI 42 (IC test_hammer_rest)")
{
    const HammerSaite h = baueVorgabeHonkytonk();
    const double cent = wertAn(h.verstimmungCent, 69, Skala::Linear);
    CHECK(440.0 * (std::exp2(cent / 2400.0) - std::exp2(-cent / 2400.0)) == doctest::Approx(2.0).epsilon(1e-3));
    // die Moden des Chors an A4: zwei Frequenzen 2 Hz auseinander (die Eigenwerte des Chors, Bank 2010)
    const auto t = hammerTeiltonModen(h, icAnschlag(69, -12, 0.5));
    REQUIRE_FALSE(t.empty());
    double lo = 1e9, hi = 0.0;
    for (const EbenenMode& m : t[0].moden)
        if (std::abs(m.anteil) > 0.05)
        {
            lo = std::min(lo, m.lambda.imag());
            hi = std::max(hi, m.lambda.imag());
        }
    CHECK((hi - lo) / (2.0 * std::numbers::pi) == doctest::Approx(2.0).epsilon(0.1));
    const HammerSaite c = baueVorgabeCp70();
    CHECK(chorAn(c, 42) == 1);
    CHECK(chorAn(c, 43) == 2);
}

TEST_CASE("InstrumentModels 34i Rechenzeit: der tiefste Ton je Instrument und das Verhaeltnis zum Fluegel (meldet nur Zahlen)")
{
    // IK2-6 a, Muster `InstrumentModels 34g Rechenzeit`: zwei Zahlen ohne Urteil — der Anteil an der Echtzeit und das
    // Verhaeltnis zum Fluegel-Kern auf derselben Maschine; die Grenze 4,6 % prueft Claude von Hand (RZ1-1 a: dreimal allein
    // nach der Taktprobe, Release-clang) — **ausgesetzt bis zum Tausch der Kuehlung** (Patrik S181). Wie IC
    // `test_hammer_rest.cpp`: der tiefste Ton, Stufe 0, 2 s (in IC 0,5 s gehalten), ohne Mitschwingen. Je Ton das Beste aus
    // drei Laeufen.
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
    // der Fluegel-Kern wie der Bezug der Grenze (`*31b-3 Rechenzeit*`): Taste 21, Stufe 0, 4 s gehalten, 4,8 s Ton
    const std::shared_ptr<const Tonkern> fluegel = kerne().erzeuge("hammerString", vorgaben(kerne().typ("hammerString")->tafel));
    REQUIRE(fluegel);
    std::vector<float> f;
    constexpr double kFluegelS = 4.8;
    const double tFluegelGanz = zeit([&] { fluegel->rechne(einzelnote(21, 27.5, 0, 4 * 48000, static_cast<Frames>(kFluegelS * 48000), 48000.0), f); });
    const double tFluegel = tFluegelGanz / kFluegelS * kDauerS;
    MESSAGE("Fluegel-Kern A0, 4 s gehalten, 4,8 s Ton: " << 100.0 * tFluegelGanz / kFluegelS << " % der Echtzeit");
    for (const Vorlage& v : vorlagen())
    {
        const HammerAnschlag a = icAnschlag(v.umfang.first, 0, 0.5);
        std::vector<float> aus;
        const double t = zeit([&] { rechneHammerTon(v.saite, a, aus, frames); });
        MESSAGE(v.name << " tiefster Ton " << v.umfang.first << ": " << 100.0 * t / kDauerS << " % der Echtzeit, " << t / tFluegel << " × Fluegel");
    }
    for (const saite::Vorlage& v : saite::vorlagen())
    {
        const saite::SaitenSpiel s = mitIcBezug(v);
        const saite::SaitenAnschlag a = icSaitenAnschlag(s.noteMin, 0);
        std::vector<float> aus;
        const double t = zeit([&] { saite::rechneSaitenTon(s, a, aus, frames); });
        MESSAGE(v.name << (v.ausgeliefert ? "" : " (nicht ausgeliefert)") << " tiefster Ton " << s.noteMin << ": " << 100.0 * t / kDauerS
                       << " % der Echtzeit, " << t / tFluegel << " × Fluegel");
    }
    CHECK(tFluegel > 0.0);
}
