/**
 ****************************************************************************************
 * @file   test_InstrumentModelsZupf.cpp
 * @brief  InstrumentModels, Stufe 34k: Klasse 2 (gezupfte Saite) — der Kern `pluckedString`
 *         (docs/audio/Instrumente_IC_Konzept.md 34k-2, GZ2 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine.
 *
 * - Tafel, Fabrik, Vorlagen, Werte hin und zurueck.
 * - Die Abdruecke aus `werte/zupf_ic.txt` (IC-Vorlage, `docs/modellierung/anbindung-ic/abdruck_zupf.cpp`) Bit fuer Bit —
 *   mit der Laufzeit und dem Bezug aus derselben Datei (der Stimmer kommt mit 34k-3), ueber den direkten Aufruf, die Clips
 *   ueber `rechneZupfClip`, und bei Stufe 0 ueber den Kern.
 * - Der Bezug: die IC-Rechnung `normiere` am uebernommenen Kern gleich den Zeilen `bezug`, `kern::normiereNachTafel` gleich
 *   den Zeilen `bezugLv`.
 * - Die IC-Faelle aus `rechnung/klassen/02_zupf_saite/test_zupf.cpp` (`4409088`) mit den IC-Grenzen; gemessen wird mit
 *   `InstrumentMeasure/ZupfStimmen` (IK2-2 a). Ohne Koto (zurueckgestellt) und ohne die Rechenzeit (kommt mit 34k-10,
 *   `*34k Rechenzeit*`; ausgesetzt bis zum Tausch der Kuehlung).
 * - Der Stimmer (34k-3): die Laute ueber `stimmeUndNormiere` Bit fuer Bit gleich der Laufzeit im Beleg, mit Bezug und dem
 *   Waechter fuer veraltete Werte; alle fuenf ueber `zupfLaufzeit`.
 * - Die zweite Tonhoehenmessung (34k-3, `InstrumentMeasure/Grundton`): an gerechneten Toenen ohne bzw. mit schwachem
 *   Grundton auf ±1 Cent; an den gestimmten Toenen der fuenf nur Zahlen (die Pruefung ±10 Cent kommt mit 34k-6).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Anschlag.hpp"
#include "InstrumentCore/Pool.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentMeasure/Bausteine.hpp"
#include "InstrumentMeasure/Grundton.hpp"
#include "InstrumentMeasure/ZupfStimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/zupf/ZupfKern.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
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
using namespace lumi::kern::zupf;

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr Frames kFrames = 28800;  // wie der Beleg: 0,6 s bei 48 kHz

double hzVon(int n) { return 440.0 * std::exp2((n - 69) / 12.0); }

double ausHex(const std::string& h)
{
    const std::uint64_t b = std::stoull(h, nullptr, 16);
    double x = 0.0;
    std::memcpy(&x, &b, sizeof x);
    return x;
}

std::string hexVon(double x)
{
    std::uint64_t b = 0;
    std::memcpy(&b, &x, sizeof b);
    char t[32];
    std::snprintf(t, sizeof t, "%016llx", static_cast<unsigned long long>(b));
    return t;
}

/// Der Beleg aus IC (`werte/zupf_ic.txt`).
struct IcBeleg
{
    std::map<std::string, std::string> abdruck;
    std::map<std::string, std::string> bezug, bezugLv;
    /// je IC-Name des Instruments die gestimmten Laufzeitfaktoren
    std::map<std::string, std::vector<NotenWert>> laufzeit;
};

const IcBeleg& icBeleg()
{
    static const IcBeleg b = [] {
        IcBeleg r;
        std::ifstream f(std::filesystem::path(__FILE__).parent_path() / "werte" / "zupf_ic.txt");
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
                if (welche == "laufzeit") r.laufzeit[name].push_back(NotenWert{note, ausHex(hex)});
            }
        }
        return r;
    }();
    return b;
}

/// Der Name des Instruments im Beleg (die IC-Namen).
std::string icName(std::string_view vorlage)
{
    if (vorlage == "harpsichord") return "Cembalo";
    if (vorlage == "concert_harp") return "Harfe";
    if (vorlage == "classical_guitar") return "Gitarre";
    if (vorlage == "lute") return "Laute";
    if (vorlage == "oud") return "Oud";
    FAIL("kein IC-Name fuer " << vorlage);
    return {};
}

const Vorlage& vorlage(std::string_view name)
{
    for (const Vorlage& v : vorlagen())
        if (v.name == name) return v;
    FAIL("keine Vorlage " << name);
    return vorlagen().front();
}

/// Die Werte einer Vorlage mit der Laufzeit aus dem Beleg (IC `stimme`); mit `icBezug` der IC-Bezug, sonst 1.
Werte gestimmteWerte(const Vorlage& v, bool icBezug)
{
    Werte w = werteAus(v.spiel);
    Kurve k;
    k.punkte = icBeleg().laufzeit.at(icName(v.name));
    k.belegkraft = Belegkraft::Rechnung;
    k.herkunft = "IC stimme (werte/zupf_ic.txt)";
    w.setze(std::string(kLaufzeit), k);
    w.setze(std::string(kBezug), icBezug ? ausHex(icBeleg().bezug.at(icName(v.name))) : 1.0);
    const std::vector<std::string> f = richteWerte(kernTyp().tafel, w);
    REQUIRE_MESSAGE(f.empty(), v.name << ": " << (f.empty() ? std::string() : f.front()));
    return w;
}

/// Das Spiel wie IC (gestimmt, mit IC-Bezug) — ueber die Tafel, wie die App es baut.
ZupfSpiel spielWieIc(std::string_view name)
{
    ZupfSpiel s = spielAus(gestimmteWerte(vorlage(name), true));
    s.name = icName(name);
    return s;
}

const ZupfSpiel& cembalo()
{
    static const ZupfSpiel c = spielWieIc("harpsichord");
    return c;
}
const ZupfSpiel& harfe()
{
    static const ZupfSpiel h = spielWieIc("concert_harp");
    return h;
}
const ZupfSpiel& gitarre()
{
    static const ZupfSpiel g = spielWieIc("classical_guitar");
    return g;
}

ZupfAnschlag ton(int note, double e = 0.5, double haltenS = 0.8)
{
    ZupfAnschlag a;
    a.note = note;
    a.staerke = e;
    a.haltenS = haltenS;
    return a;
}

std::string abdruckVon(const ZupfSpiel& s, const ZupfAnschlag& a, std::int64_t frames = kFrames)
{
    std::vector<float> t;
    rechneZupfTon(s, a, t, frames);
    return abdruck(t);
}

std::string clipAbdruck(const ZupfSpiel& s, const std::vector<ClipNote>& noten, std::int64_t frames, bool mitschwingen)
{
    std::vector<float> t;
    rechneZupfClip(s, noten, t, frames, mitschwingen);
    return abdruck(t);
}

/// Ein Ton ueber den Kern (Rechenweg Note, beginn 0) mit den Werten wie IC.
std::vector<float> kernTon(std::string_view name, int note, int stufe, Frames halten, Frames frames = kFrames)
{
    std::vector<std::string> fehler;
    const std::shared_ptr<const Tonkern> k = kerne().erzeuge(kKernName, gestimmteWerte(vorlage(name), true), &fehler);
    REQUIRE_MESSAGE(k != nullptr, (fehler.empty() ? std::string() : fehler.front()));
    const Tonauftrag a = einzelnote(note, hzVon(note), stufe, halten, frames, 48000.0);
    std::vector<float> t;
    k->rechne(a, t);
    return t;
}

double spitzeVon(const std::vector<float>& t)
{
    double s = 0.0;
    for (const float w : t) s = std::max(s, static_cast<double>(std::abs(w)));
    return s;
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < t.size(); ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * double(t[ab + i]) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

double dB(double faktor) { return 20.0 * std::log10(faktor); }

}  // namespace

TEST_CASE("InstrumentModels 34k-2 Kern pluckedString: Tafel, Fabrik, Vorlagen, Werte hin und zurueck")
{
    const KernTyp& t = kernTyp();
    const std::vector<std::string> fehler = pruefeTafel(t.tafel);
    CHECK_MESSAGE(fehler.empty(), (fehler.empty() ? std::string() : fehler.front()));
    REQUIRE(kerne().typ(kKernName) != nullptr);
    CHECK(t.tafel.bezugWert == kBezug);
    CHECK(t.tafel.umfangVon == "noteMin");
    CHECK(t.tafel.umfangBis == "noteMax");
    CHECK(t.tafel.gruppen == std::vector<std::string>{"Keys", "String", "Pluck", "Bridge", "Courses", "Sympathetic strings", "Tunable", "Output"});
    CHECK(t.auskunft.rechenweg == Rechenweg::Note);
    CHECK_FALSE(t.auskunft.koerperAnDerSchicht);
    CHECK_FALSE(t.auskunft.anregungDauernd);
    CHECK(t.auskunft.spuren.empty());
    CHECK(t.auskunft.innere.empty());
    std::vector<std::string> gestimmt;
    for (const ParameterAngabe& a : t.tafel.werte)
    {
        CAPTURE(a.name);
        CHECK_FALSE(a.herkunft.empty());
        if (a.art != WertArt::Kurve) CHECK_FALSE(a.quellen.empty());
        if (a.gestimmt)
        {
            gestimmt.push_back(a.name);
            // die mitschwingenden Saiten wirken nicht auf die Stimmung (nur im Clip)
            CHECK(std::find(a.abhaengigVon.begin(), a.abhaengigVon.end(), "sympatheticStrings") == a.abhaengigVon.end());
            CHECK(std::find(a.abhaengigVon.begin(), a.abhaengigVon.end(), "stringLength") != a.abhaengigVon.end());
        }
    }
    CHECK(gestimmt == std::vector<std::string>{std::string(kLaufzeit)});

    std::vector<std::string> namen;
    for (const Vorlage& v : vorlagen())
    {
        namen.push_back(v.name);
        CHECK_FALSE(v.ausgeliefert);  // die Dateien kommen mit 34k-9
    }
    CHECK(namen == std::vector<std::string>{"harpsichord", "concert_harp", "classical_guitar", "lute", "oud"});
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        // ungestimmt fehlt die Laufzeit (IK2-11 a: ein Fehler, kein stiller Ersatz)
        Werte roh = werteAus(v.spiel);
        CHECK(richteWerte(t.tafel, roh).size() == 1);
        // gestimmt: die Vorgaben liegen in den Grenzen, Richten aendert nichts, zurueck ins Spiel und wieder in Werte gleich
        Werte w = werteAus(v.spiel);
        const Werte g = gestimmteWerte(v, true);
        for (const std::string_view n : {kLaufzeit, kBezug}) w.setze(std::string(n), *g.finde(n));
        const Werte vorher = w;
        const std::vector<std::string> f = richteWerte(t.tafel, w);
        CHECK_MESSAGE(f.empty(), (f.empty() ? std::string() : f.front()));
        CHECK(w == vorher);
        CHECK(werteAus(spielAus(w)) == w);
    }
    // Listen und Chor: Stegmoden, Doppelchor, mitschwingende Saiten
    CHECK(cembalo().steg.size() == 10);
    CHECK(gitarre().steg.size() == 7);
    CHECK(gitarre().mitschwingNoten == std::vector<int>{40, 45, 50, 55, 59, 64});
    CHECK(harfe().mitschwingNoten.size() == 47);
    const ZupfSpiel laute = spielWieIc("lute");
    CHECK(laute.chor.saiten == 2);
    CHECK(laute.chor.verstimmungCent == 4.0);
    CHECK(laute.mitschwingNoten.empty());
    // der Zupfpunkt ist immer eine Kurve: am Cembalo 12 % → 50 %, an der Harfe flach 0,5
    CHECK(wertAn(cembalo().zupfpunktJeNote, 29, Skala::Linear) == 0.12);
    CHECK(wertAn(cembalo().zupfpunktJeNote, 89, Skala::Linear) == 0.5);
    for (const int n : {24, 60, 103}) CHECK(wertAn(harfe().zupfpunktJeNote, n, Skala::Linear) == 0.5);
}

TEST_CASE("InstrumentModels 34k-2 Kern pluckedString: die Abdruecke aus werte/zupf_ic.txt Bit fuer Bit")
{
    const IcBeleg& b = icBeleg();
    REQUIRE(b.abdruck.size() == 65);
    int geprueft = 0;
    const auto pruefe = [&](const std::string& name, const std::string& ist) {
        CAPTURE(name);
        REQUIRE(b.abdruck.count(name) == 1);
        CHECK(ist == b.abdruck.at(name));
        ++geprueft;
    };
    // die drei IC-Abdruecke (test_zupf.cpp:184–185, 242–247)
    pruefe("zupf_Cembalo_m60_mf", abdruckVon(cembalo(), ton(60, 0.5, 0.8)));
    pruefe("zupf_Gitarre_m52_mf", abdruckVon(gitarre(), ton(52, 0.5, 0.8)));
    {
        ZupfAnschlag c;
        c.note = 60;
        c.haltenS = 0.5;
        pruefe("zupf_Harfe_clip_C4_E4", clipAbdruck(harfe(), {{0.0, c}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, 48000, true));
        pruefe("zupf_Harfe_clip_C4_E4_ohne", clipAbdruck(harfe(), {{0.0, c}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, 48000, false));
    }
    // tief / mitte / hoch bei Staerke 0,2 / 0,5 / 1
    const struct
    {
        const char* name;
        double e;
    } stufen[] = {{"s02", 0.2}, {"s05", 0.5}, {"s1", 1.0}};
    for (const Vorlage& v : vorlagen())
    {
        const ZupfSpiel s = spielWieIc(v.name);
        const int mitte = (s.noteMin + s.noteMax) / 2;
        for (const int n : {s.noteMin, mitte, s.noteMax})
            for (const auto& st : stufen) pruefe("zupf_" + s.name + "_m" + std::to_string(n) + "_" + st.name, abdruckVon(s, ton(n, st.e, 0.4)));
    }
    // Spielweisen an der Mitte
    for (const ZupfSpiel* s : {&harfe(), &gitarre()})
    {
        const int m = (s->noteMin + s->noteMax) / 2;
        const std::string basis = "zupf_" + s->name + "_m" + std::to_string(m);
        ZupfAnschlag lv = ton(m, 0.5, 0.2);
        lv.laissezVibrer = true;
        pruefe(basis + "_laissezVibrer", abdruckVon(*s, lv));
        for (const int w : {0, 60})
        {
            ZupfAnschlag p = ton(m, 0.5, 0.4);
            p.polarisationGrad = w;
            pruefe(basis + "_polarisation" + std::to_string(w), abdruckVon(*s, p));
        }
    }
    pruefe("zupf_Harfe_m63_gedaempft", abdruckVon(harfe(), ton(63, 0.5, 0.2)));
    {
        ZupfAnschlag l = ton(59, 0.5, 0.4);
        l.lautenzug = true;
        pruefe("zupf_Cembalo_m59_lautenzug", abdruckVon(cembalo(), l));
        ZupfSpiel ohne = cembalo();
        ohne.tastenstoss = 0.0;
        pruefe("zupf_Cembalo_m59_s1_ohneTastenstoss", abdruckVon(ohne, ton(59, 1.0, 0.4)));
        pruefe("zupf_Cembalo_m59_s02_ohneTastenstoss", abdruckVon(ohne, ton(59, 0.2, 0.4)));
    }
    for (const std::string_view n : {"lute", "oud"})
    {
        ZupfSpiel ein = spielWieIc(n);
        const int m = (ein.noteMin + ein.noteMax) / 2;
        ein.chor = {1, 0.0};
        pruefe("zupf_" + ein.name + "_m" + std::to_string(m) + "_einChor", abdruckVon(ein, ton(m, 0.5, 0.4)));
    }
    // Clips mit Mitschwingen
    {
        ZupfAnschlag a = ton(57, 0.7, 0.3);
        pruefe("zupf_Gitarre_clip_A3_mit", clipAbdruck(gitarre(), {{0.0, a}}, 72000, true));
        pruefe("zupf_Gitarre_clip_A3_ohne", clipAbdruck(gitarre(), {{0.0, a}}, 72000, false));
        std::vector<ClipNote> akkord;
        const int griff[] = {40, 47, 52, 56, 59, 64};
        for (int i = 0; i < 6; ++i) akkord.push_back({0.015 * i, ton(griff[i], 0.6, 0.6)});
        pruefe("zupf_Gitarre_clip_EDur_mit", clipAbdruck(gitarre(), akkord, 72000, true));
    }
    {
        bool ok = false;
        const Pedale d = pedaleVorschlag({62, 64, 66, 67, 69, 71, 73}, &ok);
        REQUIRE(ok);
        pruefe("zupf_Harfe_clip_DDur_mit",
               clipAbdruck(harfeMitPedalen(harfe(), d), {{0.0, ton(62, 0.5, 0.5)}, {0.2, ton(66, 0.5, 0.5)}, {0.4, ton(69, 0.5, 0.5)}}, 48000, true));
    }
    CHECK(geprueft == 65);

    // ueber den Kern bei Stufe 0 (Staerke 1): tief / mitte / hoch, 0,4 s gehalten
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const int mitte = (v.spiel.noteMin + v.spiel.noteMax) / 2;
        for (const int n : {v.spiel.noteMin, mitte, v.spiel.noteMax})
            CHECK(abdruck(kernTon(v.name, n, 0, 19200)) == b.abdruck.at("zupf_" + icName(v.name) + "_m" + std::to_string(n) + "_s1"));
    }
}

TEST_CASE("InstrumentModels 34k-2 Kern pluckedString: Staerke aus der Stufe, Nachklang")
{
    const ZupfSpiel& c = cembalo();
    const ZupfSpiel& g = gitarre();
    CHECK(staerkeBei(g, 0) == 1.0);
    CHECK(staerkeBei(g, 3) == 1.0);
    // die Kraft folgt der Stufe in dB: kraftPp · (kraftFf/kraftPp)^e = kraftFf · 10^(stufe/20)
    for (const int stufe : {-1, -6, -12, -18})
    {
        CAPTURE(stufe);
        const double e = staerkeBei(g, stufe);
        CHECK(g.kraftPpN * std::pow(g.kraftFfN / g.kraftPpN, e) == doctest::Approx(g.kraftFfN * anschlagFaktor(stufe)).epsilon(1e-12));
    }
    // unter forcePp bleibt die Staerke 0 (Gitarre 0,5 … 4 N: 18,1 dB; Cembalo 0,6 … 1 N: 4,4 dB)
    CHECK(staerkeBei(g, -19) == 0.0);
    CHECK(staerkeBei(c, -5) == 0.0);
    CHECK(staerkeBei(c, -4) > 0.0);
    // ueber den Kern: Stufe −6 wie der direkte Aufruf mit staerkeBei
    {
        const int m = (g.noteMin + g.noteMax) / 2;
        CHECK(abdruck(kernTon("classical_guitar", m, -6, 19200)) == abdruckVon(g, ton(m, staerkeBei(g, -6), 0.4)));
    }
    // der Nachklang deckt Daempfen und Stegmoden: nach dem Ende von auftrag.frames ist der Ton 60 dB unter der Spitze
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        std::vector<std::string> fehler;
        const std::shared_ptr<const Tonkern> k = kerne().erzeuge(kKernName, gestimmteWerte(v, true), &fehler);
        REQUIRE(k != nullptr);
        const int m = (v.spiel.noteMin + v.spiel.noteMax) / 2;
        const Frames halten = 9600;
        const Frames rest = k->nachklang(einzelnote(m, hzVon(m), 0, halten, 0, 48000.0), 10 * 48000);
        CHECK(rest > static_cast<Frames>(v.spiel.daempfenS * 48000.0));
        CHECK(rest < 48000);
        // laenger gerechnet: was nach halten + rest kommt, liegt 60 dB unter der Spitze
        const std::vector<float> t = kernTon(v.name, m, 0, halten, halten + rest + 48000);
        double danach = 0.0;
        for (std::size_t i = static_cast<std::size_t>(halten + rest); i + 2400 < t.size(); ++i) danach = std::max(danach, double(std::abs(t[i])));
        MESSAGE(v.name << ": Nachklang " << 1000.0 * rest / 48000.0 << " ms, danach " << dB(danach / spitzeVon(t)) << " dB");
        CHECK(danach < 1e-3 * spitzeVon(t));
    }
}

TEST_CASE("InstrumentModels 34k-2 Kern pluckedString: der Bezug nach IC und nach der Tafel")
{
    const IcBeleg& b = icBeleg();
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        // IC normiere (Vorgaben.cpp:54–67) am uebernommenen Kern: Mitte, Staerke 1, 0,5 s
        {
            ZupfSpiel s = spielAus(gestimmteWerte(v, false));
            ZupfAnschlag a;
            a.note = (s.noteMin + s.noteMax) / 2;
            a.staerke = 1.0;
            a.haltenS = 0.5;
            std::vector<float> t;
            rechneZupfTon(s, a, t, a.rate / 2);
            const double sp = spitzeVon(t);
            CHECK(hexVon(sp > 0.0 ? sp / 0.5 : 1.0) == b.bezug.at(icName(v.name)));
        }
        // die Probe von LumiViz (IK2-10 a)
        std::vector<std::string> fehler;
        const double bezug = normiereNachTafel(kerne(), kKernName, gestimmteWerte(v, false), &fehler);
        CHECK(fehler.empty());
        CHECK(hexVon(bezug) == b.bezugLv.at(icName(v.name)));
        CHECK(b.bezugLv.at(icName(v.name)) == b.bezug.at(icName(v.name)));  // J76: bei allen fuenf gleich
    }
}

TEST_CASE("InstrumentModels 34k-2 IC test_zupf: jede fuenfte Note aller Instrumente auf ±5 Cent (Spitze des Grundtons)")
{
    for (const Vorlage& v : vorlagen())
    {
        const ZupfSpiel s = spielWieIc(v.name);
        for (int note = s.noteMin; note <= s.noteMax; note += 5)
        {
            CAPTURE(v.name);
            CAPTURE(note);
            const lumi::messung::ZupfTon m = lumi::messung::messeZupfTon(s, ton(note), 28800);
            REQUIRE(m.schwingt);
            CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(note))) < 5.0);
        }
    }
    // die Messung liest nicht ueber das Ende hinaus (IC tat es bei weniger als 0,1 s)
    CHECK_FALSE(lumi::messung::messeZupfTon(gitarre(), ton(52), 4000).schwingt);
}

TEST_CASE("InstrumentModels 34k-2 IC test_zupf: zwei Polarisationen am Steg — mit Kopplung zweistufiges Abklingen (Bank & Karjalainen 2010)")
{
    const ZupfSpiel& g = gitarre();
    ZupfSpiel eben = g;
    for (StegMode& m : eben.steg) m.winkelGrad = 0.0;
    const auto raten = [](const ZupfSpiel& s, double winkel) {
        ZupfAnschlag a = ton(55, 0.6, 3.0);
        a.polarisationGrad = winkel;
        std::vector<float> t;
        rechneZupfTon(s, a, t, 144000);
        const double f = hzVon(55);
        const double a0 = band(t, 4800, 4800, f), a1 = band(t, 28800, 4800, f), a2 = band(t, 72000, 4800, f), a3 = band(t, 120000, 4800, f);
        const double frueh = std::log(a0 / a1) / 0.5, spaet = std::log(a2 / a3) / 1.0;
        return frueh / spaet;
    };
    const double einfach = raten(eben, 0.0), zweifach = raten(g, 45.0);
    MESSAGE("Gitarre G3: Abklingrate frueh/spaet — eine Polarisation " << einfach << ", zwei gekoppelt " << zweifach);
    CHECK(zweifach > 2.0 * einfach);
}

TEST_CASE("InstrumentModels 34k-2 IC test_zupf: Cembalo — Zupfpunkt (Valimaki), Lautenzug, Tastenstoss (Penttinen)")
{
    std::vector<float> diskant, bass;
    rechneZupfTon(cembalo(), ton(89), diskant, 28800);
    rechneZupfTon(cembalo(), ton(31), bass, 28800);
    const double d = dB(band(diskant, 2400, 16384, 2.0 * hzVon(89)) / band(diskant, 2400, 16384, hzVon(89)));
    const double b = dB(band(bass, 2400, 16384, 2.0 * hzVon(31)) / band(bass, 2400, 16384, hzVon(31)));
    MESSAGE("Cembalo: H2/H1 Diskant f3 " << d << " dB, Bass G1 " << b << " dB");
    CHECK(d < b - 10.0);

    std::vector<float> frei, laute;
    rechneZupfTon(cembalo(), ton(60, 0.5, 2.0), frei, 96000);
    ZupfAnschlag l = ton(60, 0.5, 2.0);
    l.lautenzug = true;
    rechneZupfTon(cembalo(), l, laute, 96000);
    CHECK(band(laute, 48000, 8192, hzVon(60)) < 0.3 * band(frei, 48000, 8192, hzVon(60)));
    std::vector<float> pp, ff;
    rechneZupfTon(cembalo(), ton(60, 0.0), pp, 24000);
    rechneZupfTon(cembalo(), ton(60, 1.0), ff, 24000);
    double ep = 0.0, ef = 0.0;
    for (double f = 30.0; f < 300.0; f += 5.0)
    {
        ep += std::pow(band(pp, 0, 4800, f), 2);
        ef += std::pow(band(ff, 0, 4800, f), 2);
    }
    MESSAGE("Cembalo: unter 300 Hz ff gegen pp " << 10.0 * std::log10(ef / ep) << " dB");
    CHECK(ef > ep);
}

TEST_CASE("InstrumentModels 34k-2 IC test_zupf: Harfe laissez vibrer, sonst daempft die Hand; Laute mit Doppelchor")
{
    std::vector<float> gedaempft, frei;
    rechneZupfTon(harfe(), ton(60, 0.5, 0.5), gedaempft, 48000);
    ZupfAnschlag lv = ton(60, 0.5, 0.5);
    lv.laissezVibrer = true;
    rechneZupfTon(harfe(), lv, frei, 48000);
    CHECK(band(gedaempft, 36000, 8192, hzVon(60)) < 0.1 * band(frei, 36000, 8192, hzVon(60)));
    std::vector<float> t;
    rechneZupfTon(spielWieIc("lute"), ton(60, 0.6, 3.0), t, 144000);
    CHECK(t.size() == 144000u);
    CHECK(spitzeVon(t) > 0.0);
}

TEST_CASE("InstrumentModels 34k-2 IC test_zupf: je Clip — eine Saite wie je Note, die leeren Saiten schwingen mit, Harfe mit 47 Saiten")
{
    const ZupfSpiel& g = gitarre();
    const ZupfAnschlag a = ton(57, 0.7, 0.3);
    std::vector<float> je, clip;
    rechneZupfTon(g, a, je, 48000);
    rechneZupfClip(g, {{0.0, a}}, clip, 48000, false);
    double maxAbw = 0.0, maxWert = 0.0;
    for (std::size_t i = 0; i < je.size(); ++i)
    {
        maxAbw = std::max(maxAbw, std::abs(double(je[i]) - clip[i]));
        maxWert = std::max(maxWert, std::abs(double(je[i])));
    }
    CHECK(maxAbw < 1e-5 * maxWert);
    std::vector<float> mit, ohne;
    rechneZupfClip(g, {{0.0, a}}, mit, 72000, true);
    rechneZupfClip(g, {{0.0, a}}, ohne, 72000, false);
    const double m220 = band(mit, 33600, 24000, 220.0), o220 = band(ohne, 33600, 24000, 220.0);
    MESSAGE("Gitarre A3 gedaempft, 0,7–1,2 s bei 220 Hz: mit leeren Saiten " << 20.0 * std::log10(m220 / std::max(1e-30, o220)) << " dB gegen ohne");
    CHECK(m220 > 10.0 * o220);
    const ZupfSpiel& h = harfe();
    CHECK(h.mitschwingNoten.size() == 47u);
    ZupfAnschlag c;
    c.note = 60;
    c.haltenS = 0.5;
    std::vector<float> h1, h2;
    rechneZupfClip(h, {{0.0, c}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, h1, 48000);
    rechneZupfClip(h, {{0.0, c}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, h2, 48000);
    CHECK(h1 == h2);
    CHECK(spitzeVon(h1) > 0.0);
}

TEST_CASE("InstrumentModels 34k-2 IC test_zupf: Harfe Pedale — Diagramm, Vorschlag, Bisbigliando, Glissando, Spur, nicht spielbar")
{
    const ZupfSpiel& h = harfe();
    const Pedale cDur{0, 0, 0, 0, 0, 0, 0};
    CHECK(harfenSaiten(h, cDur) == h.mitschwingNoten);
    bool ok = false;
    const Pedale d = pedaleVorschlag({62, 64, 66, 67, 69, 71, 73}, &ok);
    CHECK(ok);
    CHECK(d == Pedale{1, 0, 0, 1, 0, 0, 0});
    const Pedale eis{0, 0, 1, 0, 0, 0, 0};
    CHECK(saitenFuer(65, eis).size() == 2u);
    CHECK_FALSE(spielbar(64, eis));
    const std::vector<int> gl = glissandoNoten(h, 60, 72, cDur);
    CHECK(gl == std::vector<int>{60, 62, 64, 65, 67, 69, 71, 72});
    CHECK(glissandoNoten(h, 72, 60, cDur).front() == 72);
    (void)pedaleVorschlag({60, 61, 62, 63, 64, 65, 66, 67}, &ok);
    CHECK_FALSE(ok);
    std::vector<ClipNote> chromatisch;
    for (int n = 60; n <= 72; ++n)
    {
        ZupfAnschlag a;
        a.note = n;
        chromatisch.push_back({0.25 * (n - 60), a});
    }
    const std::vector<PedalStelle> spur = pedalSpurVorschlag(chromatisch);
    CHECK(spur.size() > 1u);
    for (const ClipNote& cn : chromatisch)
    {
        const PedalStelle* gilt = &spur.front();
        for (const PedalStelle& st : spur)
            if (st.abS <= cn.abS) gilt = &st;
        CAPTURE(cn.anschlag.note);
        CHECK(spielbar(cn.anschlag.note, gilt->pedale));
    }
    const ZupfSpiel hd = harfeMitPedalen(h, d);
    CHECK(std::find(hd.mitschwingNoten.begin(), hd.mitschwingNoten.end(), 66) != hd.mitschwingNoten.end());
    CHECK(std::find(hd.mitschwingNoten.begin(), hd.mitschwingNoten.end(), 65) == hd.mitschwingNoten.end());
}

TEST_CASE("InstrumentModels 34k-2 IC test_zupf: Determinismus auf Threads")
{
    const ZupfSpiel& g = gitarre();
    std::vector<float> a;
    rechneZupfTon(g, ton(52), a, 28800);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneZupfTon(g, ton(52), q[i], 28800); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
    CHECK(abdruck(a) == icBeleg().abdruck.at("zupf_Gitarre_m52_mf"));
}

TEST_CASE("InstrumentModels 34k-3 Stimmer pluckedString: Laute Bit fuer Bit wie IC, Bezug, Waechter fuer veraltete Werte")
{
    const std::vector<std::string> eins = {std::string(kLaufzeit)};
    REQUIRE(lumi::messung::stimmerFuer(lumi::messung::stimmer(), kKernName) != nullptr);
    Werte w = werteAus(vorlage("lute").spiel);
    const auto beginn = std::chrono::steady_clock::now();
    const lumi::messung::StimmBericht b = lumi::messung::stimmeUndNormiere(kerne(), lumi::messung::stimmer(), kKernName, w, true, true);
    const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
    MESSAGE("Laute gestimmt und normiert in " << dauer << " s auf " << poolThreads() << " Threads (13 Stuetzpunkte)");
    CHECK_MESSAGE(b.fehler.empty(), (b.fehler.empty() ? std::string() : b.fehler.front()));
    CHECK(b.gestimmt == eins);
    REQUIRE(b.mitBezug);
    CHECK(hexVon(b.bezug) == icBeleg().bezugLv.at("Laute"));
    CHECK(w.kurve(kLaufzeit).punkte == icBeleg().laufzeit.at("Laute"));
    CHECK(w.kurve(kLaufzeit).belegkraft == Belegkraft::Rechnung);
    CHECK(richteWerte(kernTyp().tafel, w).empty());
    // der Waechter (IK2-11 a): frisch gestimmt nichts veraltet, der Bezug passt
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, w).empty());
    CHECK_FALSE(lumi::messung::bezugVeraltet(kerne(), kKernName, w));
    // absichtlich veraltet: eine Eingabe der Saite geaendert
    Werte alt = w;
    alt.setze("stringQ", alt.zahl("stringQ") * 1.5);
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, alt) == eins);
    // die mitschwingenden Saiten wirken nicht auf die Stimmung — sie veralten nichts
    Werte mit = w;
    Liste l;
    Werte e;
    e.felder.emplace_back("note", std::int64_t{55});
    l.eintraege.push_back(e);
    mit.setze("sympatheticStrings", l);
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, mit).empty());
    // ein fremder Abdruck genuegt
    Werte fremd = w;
    fremd.merkeAbdruck(std::string(kLaufzeit), "0000");
    CHECK(lumi::messung::veralteteWerte(kerne(), kKernName, fremd) == eins);
    // ein falscher Bezug faellt auf
    Werte bezug = w;
    bezug.setze(std::string(kBezug), b.bezug * 1.01);
    CHECK(lumi::messung::bezugVeraltet(kerne(), kKernName, bezug));
}

TEST_CASE("InstrumentModels 34k-3 Stimmer pluckedString: alle fuenf Bit fuer Bit wie IC stimme")
{
    double gesamt = 0.0;
    for (const Vorlage& v : vorlagen())
    {
        CAPTURE(v.name);
        const auto beginn = std::chrono::steady_clock::now();
        const Kurve k = lumi::messung::zupfLaufzeit(v.spiel);
        const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - beginn).count();
        gesamt += dauer;
        MESSAGE(v.name << ": " << k.punkte.size() << " Stuetzpunkte in " << dauer << " s");
        CHECK(k.punkte == icBeleg().laufzeit.at(icName(v.name)));
    }
    MESSAGE("alle fuenf in " << gesamt << " s auf " << poolThreads() << " Threads");
}

namespace
{

/// Ein gerechneter Ton aus Teiltoenen: f_k = k·f₀·√(1 + B·k²), Amplitude a_k, Abklingzeit τ_k, Phase fest; dazu Rauschen
/// (Gleichverteilung aus einer festen Folge) mit der Spitze `rauschen`.
std::vector<double> teiltonTon(double f0, double steife, const std::vector<std::pair<int, double>>& teiltoene, double rauschen, double sekunden = 0.6)
{
    const int rate = 48000;
    std::vector<double> t(static_cast<std::size_t>(sekunden * rate), 0.0);
    for (const auto& [k, a] : teiltoene)
    {
        const double f = k * f0 * std::sqrt(1.0 + steife * k * k);
        const double tau = 1.5 / std::sqrt(double(k));
        for (std::size_t i = 0; i < t.size(); ++i)
        {
            const double s = double(i) / rate;
            t[i] += a * std::exp(-s / tau) * std::sin(2.0 * kPi * f * s + 0.7 * k);
        }
    }
    std::uint64_t z = 0x9e3779b97f4a7c15ull;
    for (double& x : t)
    {
        z = z * 6364136223846793005ull + 1442695040888963407ull;
        x += rauschen * (double(z >> 11) / double(1ull << 53) * 2.0 - 1.0);
    }
    return t;
}

double centZwischen(double ist, double soll) { return 1200.0 * std::log2(ist / soll); }

}  // namespace

TEST_CASE("InstrumentModels 34k-3 zweite Tonhoehenmessung: Grundton aus den Teiltoenen ohne und mit schwachem Grundton")
{
    using lumi::messung::grundtonAusTeiltoenen;
    // (1) ohne Grundton: G2 + 13,7 Cent, Teiltoene 2 … 10, Rauschen −80 dB
    {
        const double f0 = hzVon(43) * std::exp2(13.7 / 1200.0);
        std::vector<std::pair<int, double>> tt;
        for (int k = 2; k <= 10; ++k) tt.emplace_back(k, 1.0 / k);
        const std::vector<double> t = teiltonTon(f0, 0.0, tt, 1e-4);
        const lumi::messung::Grundton g = grundtonAusTeiltoenen(t, 48000, hzVon(43));
        REQUIRE(g.gefunden);
        MESSAGE("ohne Grundton: " << centZwischen(g.hz, f0) << " Cent daneben aus " << g.teiltoene << " Teiltoenen, B = " << g.steife);
        CHECK(std::abs(centZwischen(g.hz, f0)) < 1.0);
        CHECK(g.teiltoene == 9);
        // die Messung des Stimmers (IC) sucht nur nahe der Note: ohne Grundton misst sie dort Rauschen und Nebenkeulen (nur Zahl)
        const lumi::messung::ZupfTon ic = lumi::messung::messeZupfSumme(t, 43, 48000);
        MESSAGE("die Messung des Stimmers am selben Ton: " << centZwischen(ic.spielHz, f0) << " Cent");
        // und YIN (B2) als Median der stimmhaften Rahmen
        const lumi::messung::Tonhoehe y = lumi::messung::tonhoehe(t, 48000, 30.0, 3000.0);
        std::vector<double> hz;
        for (const double x : y.hz)
            if (x > 0.0) hz.push_back(x);
        if (!hz.empty())
        {
            std::nth_element(hz.begin(), hz.begin() + hz.size() / 2, hz.end());
            MESSAGE("YIN am selben Ton: " << centZwischen(hz[hz.size() / 2], f0) << " Cent (Median)");
        }
    }
    // (2) schwacher Grundton (−40 dB) an einer steifen Saite: A1 − 21,3 Cent, B = 3·10⁻⁴, Teiltoene 1 … 12
    {
        const double f0 = hzVon(33) * std::exp2(-21.3 / 1200.0);
        std::vector<std::pair<int, double>> tt = {{1, 0.01}};
        for (int k = 2; k <= 12; ++k) tt.emplace_back(k, 1.0 / k);
        const std::vector<double> t = teiltonTon(f0, 3e-4, tt, 1e-4);
        const lumi::messung::Grundton g = grundtonAusTeiltoenen(t, 48000, hzVon(33));
        REQUIRE(g.gefunden);
        MESSAGE("schwacher Grundton, steif: " << centZwischen(g.hz, f0) << " Cent daneben aus " << g.teiltoene << " Teiltoenen, B = " << g.steife);
        CHECK(std::abs(centZwischen(g.hz, f0)) < 1.0);
        CHECK(g.steife == doctest::Approx(3e-4).epsilon(0.1));
    }
    // (3) die Schaetzung 40 Cent daneben: der zweite Durchgang findet den Ton
    {
        const double f0 = hzVon(57);
        std::vector<std::pair<int, double>> tt;
        for (int k = 2; k <= 8; ++k) tt.emplace_back(k, 1.0);
        const std::vector<double> t = teiltonTon(f0, 0.0, tt, 1e-4);
        const lumi::messung::Grundton g = grundtonAusTeiltoenen(t, 48000, f0 * std::exp2(40.0 / 1200.0));
        REQUIRE(g.gefunden);
        CHECK(std::abs(centZwischen(g.hz, f0)) < 1.0);
    }
    // (4) ohne Signal nichts
    CHECK_FALSE(grundtonAusTeiltoenen(std::vector<double>(28800, 0.0), 48000, 110.0).gefunden);
    CHECK_FALSE(grundtonAusTeiltoenen(std::vector<double>(2000, 0.0), 48000, 110.0).gefunden);
}

TEST_CASE("InstrumentModels 34k-3 zweite Tonhoehenmessung an den gestimmten Toenen der fuenf (nur Zahlen)")
{
    // die Pruefung ±10 Cent an jeder Taste kommt mit 34k-6; hier, wie weit die IC-Stimmung nach der zweiten Messung liegt
    for (const Vorlage& v : vorlagen())
    {
        const ZupfSpiel s = spielWieIc(v.name);
        const int n = s.noteMax - s.noteMin + 1;
        std::vector<double> cent(static_cast<std::size_t>(n), 0.0);
        std::vector<int> zahl(static_cast<std::size_t>(n), 0);
        parallelJe(static_cast<std::size_t>(n), [&](std::size_t i) {
            const int note = s.noteMin + static_cast<int>(i);
            std::vector<double> summe;
            rechneZupfSumme(s, ton(note, 0.5, 0.6), summe, 28800);
            const lumi::messung::Grundton g = lumi::messung::grundtonAusTeiltoenen(summe, 48000, hzVon(note));
            cent[i] = g.gefunden ? centZwischen(g.hz, hzVon(note)) : 1e9;
            zahl[i] = g.teiltoene;
        });
        double schlimmste = 0.0;
        int wo = s.noteMin, ueber10 = 0, ohne = 0;
        for (int i = 0; i < n; ++i)
        {
            if (cent[static_cast<std::size_t>(i)] > 1e8)
            {
                ++ohne;
                continue;
            }
            if (std::abs(cent[static_cast<std::size_t>(i)]) > 10.0) ++ueber10;
            if (std::abs(cent[static_cast<std::size_t>(i)]) > std::abs(schlimmste))
            {
                schlimmste = cent[static_cast<std::size_t>(i)];
                wo = s.noteMin + i;
            }
        }
        MESSAGE(v.name << ": " << n << " Tasten, schlimmste " << schlimmste << " Cent an " << wo << ", ueber 10 Cent " << ueber10 << ", ohne Ergebnis "
                       << ohne);
        CHECK(ohne == 0);
    }
}
