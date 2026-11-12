/**
 * @file abdruck_zupf.cpp
 * @brief Beleg 34k-1 (S194): die Abdruecke der Klasse 2 aus der IC-Vorlage, ohne Aenderung an der Vorlage
 *
 * IC fuehrt fuer die Klasse 2 drei Abdruecke (`zupf_Cembalo_m60_mf`, `zupf_Gitarre_m52_mf`, `zupf_Harfe_clip_C4_E4` in
 * `werte/abdruck_msvc.txt`). Dieses Programm rechnet mit der Vorlage unter `harvest/instruments-calculation/rechnung`
 * (IC `4409088`) unter MSVC (`/MD`, die Plattform von `abdruck_msvc.txt`) die uebrigen, die LumiViz in 34k-2 und 34k-3
 * Bit fuer Bit nachbilden muss (Instrumente_IC_Konzept.md, Teilplan 34k, GZ2 a). Es gehoert zu keinem Bau; der Baubefehl
 * steht in `abdruck_zupf.bat` daneben, die Ausgabe ist `tests/unit/UnitTests/werte/zupf_ic.txt`.
 *
 * Inhalt:
 * - die drei IC-Abdruecke (test_zupf.cpp:147–186, 233–247) als Gegenprobe zu `abdruck_msvc.txt`;
 * - je Instrument (Cembalo, Harfe, Gitarre, Laute, Oud) tief / mitte / hoch des Umfangs bei Staerke 0,2 / 0,5 / 1,
 *   0,4 s gehalten, 28 800 Frames (mit dem Daempfen und dem Ausklang);
 * - an der Mitte: Harfe und Gitarre *laissez vibrer*, Cembalo Lautenzug und ohne Tastenstoss, Laute und Oud mit einem
 *   Chor statt des Doppelchors, Gitarre und Harfe mit Polarisation 0° und 60° statt der Vorgabe;
 * - Clips (`rechneZupfClip`): Gitarre A3 mit und ohne Mitschwingen (IC test_zupf), Gitarre ein Akkord E-Dur mit
 *   Mitschwingen; Harfe C4 / E4 ohne Mitschwingen; Harfe unter D-Dur (`harfeMitPedalen`, F♯ und C♯) mit D4, F♯4, A4;
 * - je Instrument der Bezug `ausgangBezug` aus IC `normiere` (Mitte, Staerke 1, 0,5 s) und mit der Probe von LumiViz
 *   (`kern::bezugsProbe`: Mitte von noteMin … noteMax, Staerke 1, 1 s gehalten und gerechnet);
 * - je Instrument die gestimmten Laufzeitfaktoren aus `stimme` (`laufzeitJeNote`) Punkt fuer Punkt in Hex.
 */
#include "klassen/02_zupf_saite/Zupf.hpp"
#include "gemeinsam/Werkzeug.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace ic;
using namespace ic::zupf;

namespace
{

constexpr std::int64_t kFrames = 28800;  // 0,6 s bei 48 kHz

std::string hexVon(double x)
{
    std::uint64_t b = 0;
    std::memcpy(&b, &x, sizeof b);
    char t[32];
    std::snprintf(t, sizeof t, "%016llx", static_cast<unsigned long long>(b));
    return t;
}

/// IC `normiere` (klassen/02_zupf_saite/Vorgaben.cpp:54–67) ohne `stimme` (die Vorgabe ist schon gestimmt), mit
/// waehlbarer Laenge von Halten und Rechnung.
double bezugMit(ZupfSpiel s, double haltenS, std::int64_t frames)
{
    s.ausgangBezug = 1.0;
    ZupfAnschlag a;
    a.note = (s.noteMin + s.noteMax) / 2;
    a.staerke = 1.0;
    a.haltenS = haltenS;
    std::vector<float> ton;
    rechneZupfTon(s, a, ton, frames);
    const double sp = spitze(ton);
    return sp > 0.0 ? sp / 0.5 : 1.0;
}

ZupfAnschlag ton(int note, double e, double haltenS = 0.4)
{
    ZupfAnschlag a;
    a.note = note;
    a.staerke = e;
    a.haltenS = haltenS;
    return a;
}

void zeile(const std::string& name, const ZupfSpiel& s, const ZupfAnschlag& a, std::int64_t frames = kFrames)
{
    std::vector<float> t;
    rechneZupfTon(s, a, t, frames);
    std::printf("abdruck %s %s\n", name.c_str(), abdruck(t).c_str());
}

void clip(const std::string& name, const ZupfSpiel& s, const std::vector<ClipNote>& noten, std::int64_t frames, bool mitschwingen)
{
    std::vector<float> t;
    rechneZupfClip(s, noten, t, frames, mitschwingen);
    std::printf("abdruck %s %s\n", name.c_str(), abdruck(t).c_str());
}

void kurve(const char* instrument, const char* name, const Kurve& k)
{
    for (const auto& p : k.punkte)
        std::printf("kurve %s %s %d %s %.17g\n", instrument, name, p.note, hexVon(p.wert).c_str(), p.wert);
}

}  // namespace

int main()
{
    std::printf("# Bezuege der Klasse 2 aus InstrumentsCalculation (4409088, Vorlage unter harvest/), gerechnet 34k-1 (S194)\n");
    std::printf("# mit docs/modellierung/anbindung-ic/abdruck_zupf.cpp unter MSVC (/MD) — nicht von Hand aendern\n");
    std::printf("# abdruck <name> <sha256>: 28800 Frames bei 48 kHz; _s02/_s05/_s1 = Staerke; 0,4 s gehalten, ausser\n");
    std::printf("#   den drei IC-Abdruecken (test_zupf: Staerke 0,5, 0,8 s gehalten) und den Clips (Laenge im Namen nicht,\n");
    std::printf("#   siehe abdruck_zupf.cpp)\n");
    std::printf("# bezug <name> <hex der 64 Bit> <Zahl>: ausgangBezug nach IC normiere (Mitte, Staerke 1, 0,5 s)\n");
    std::printf("# bezugLv <name> <hex> <Zahl>: dieselbe Rechnung mit der Probe von LumiViz (Mitte, Staerke 1, 1 s)\n");
    std::printf("# kurve <name> laufzeit <note> <hex> <Zahl>: die gestimmten Faktoren aus stimme (laufzeitJeNote)\n");

    struct Fall
    {
        const char* name;
        ZupfSpiel s;
    };
    const std::vector<Fall> faelle = {{"Cembalo", baueVorgabeCembalo()},
                                      {"Harfe", baueVorgabeHarfe()},
                                      {"Gitarre", baueVorgabeGitarre()},
                                      {"Laute", baueVorgabeLaute()},
                                      {"Oud", baueVorgabeOud()}};
    const ZupfSpiel& cembalo = faelle[0].s;
    const ZupfSpiel& harfe = faelle[1].s;
    const ZupfSpiel& gitarre = faelle[2].s;
    const auto mitteVon = [](const ZupfSpiel& s) { return (s.noteMin + s.noteMax) / 2; };

    // die drei IC-Abdruecke (test_zupf.cpp:184–185, 242–247)
    zeile("zupf_Cembalo_m60_mf", cembalo, ton(60, 0.5, 0.8));
    zeile("zupf_Gitarre_m52_mf", gitarre, ton(52, 0.5, 0.8));
    {
        ZupfAnschlag c;
        c.note = 60;
        c.haltenS = 0.5;
        clip("zupf_Harfe_clip_C4_E4", harfe, {{0.0, c}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, 48000, true);
    }

    // tief / mitte / hoch bei Staerke 0,2 / 0,5 / 1
    const struct
    {
        const char* name;
        double e;
    } stufen[] = {{"s02", 0.2}, {"s05", 0.5}, {"s1", 1.0}};
    for (const Fall& f : faelle)
    {
        const int noten[] = {f.s.noteMin, mitteVon(f.s), f.s.noteMax};
        for (const int n : noten)
            for (const auto& st : stufen)
                zeile("zupf_" + std::string(f.name) + "_m" + std::to_string(n) + "_" + st.name, f.s, ton(n, st.e));
    }

    // Spielweisen an der Mitte
    {
        const int m = mitteVon(harfe);
        ZupfAnschlag lv = ton(m, 0.5, 0.2);
        lv.laissezVibrer = true;
        zeile("zupf_Harfe_m" + std::to_string(m) + "_laissezVibrer", harfe, lv);
        zeile("zupf_Harfe_m" + std::to_string(m) + "_gedaempft", harfe, ton(m, 0.5, 0.2));
        for (const double w : {0.0, 60.0})
        {
            ZupfAnschlag p = ton(m, 0.5);
            p.polarisationGrad = w;
            zeile("zupf_Harfe_m" + std::to_string(m) + "_polarisation" + std::to_string(static_cast<int>(w)), harfe, p);
        }
    }
    {
        const int m = mitteVon(gitarre);
        ZupfAnschlag lv = ton(m, 0.5, 0.2);
        lv.laissezVibrer = true;
        zeile("zupf_Gitarre_m" + std::to_string(m) + "_laissezVibrer", gitarre, lv);
        for (const double w : {0.0, 60.0})
        {
            ZupfAnschlag p = ton(m, 0.5);
            p.polarisationGrad = w;
            zeile("zupf_Gitarre_m" + std::to_string(m) + "_polarisation" + std::to_string(static_cast<int>(w)), gitarre, p);
        }
    }
    {
        const int m = mitteVon(cembalo);
        ZupfAnschlag l = ton(m, 0.5);
        l.lautenzug = true;
        zeile("zupf_Cembalo_m" + std::to_string(m) + "_lautenzug", cembalo, l);
        ZupfSpiel ohne = cembalo;
        ohne.tastenstoss = 0.0;
        zeile("zupf_Cembalo_m" + std::to_string(m) + "_s1_ohneTastenstoss", ohne, ton(m, 1.0));
        zeile("zupf_Cembalo_m" + std::to_string(m) + "_s02_ohneTastenstoss", ohne, ton(m, 0.2));
    }
    for (const Fall& f : {faelle[3], faelle[4]})
    {
        ZupfSpiel ein = f.s;
        ein.chor = {1, 0.0};
        zeile("zupf_" + std::string(f.name) + "_m" + std::to_string(mitteVon(f.s)) + "_einChor", ein, ton(mitteVon(f.s), 0.5));
    }

    // Clips mit Mitschwingen
    {
        ZupfAnschlag a;
        a.note = 57;  // A3, wie IC test_zupf.cpp:150–153
        a.staerke = 0.7;
        a.haltenS = 0.3;
        clip("zupf_Gitarre_clip_A3_mit", gitarre, {{0.0, a}}, 72000, true);
        clip("zupf_Gitarre_clip_A3_ohne", gitarre, {{0.0, a}}, 72000, false);
        // E-Dur, gestrichen (E2 H2 E3 G♯3 H3 E4 im Abstand 15 ms)
        std::vector<ClipNote> akkord;
        const int griff[] = {40, 47, 52, 56, 59, 64};
        for (int i = 0; i < 6; ++i) akkord.push_back({0.015 * i, ton(griff[i], 0.6, 0.6)});
        clip("zupf_Gitarre_clip_EDur_mit", gitarre, akkord, 72000, true);
    }
    {
        ZupfAnschlag c;
        c.note = 60;
        c.haltenS = 0.5;
        clip("zupf_Harfe_clip_C4_E4_ohne", harfe, {{0.0, c}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, 48000, false);
        // D-Dur (F♯, C♯): die mitschwingenden Saiten neu gestimmt
        bool ok = false;
        const Pedale d = pedaleVorschlag({62, 64, 66, 67, 69, 71, 73}, &ok);
        const ZupfSpiel hd = harfeMitPedalen(harfe, d);
        std::printf("# Harfe D-Dur: Pedale %d %d %d %d %d %d %d (ok %d), %zu mitschwingende Saiten\n", d[0], d[1], d[2], d[3], d[4], d[5],
                    d[6], ok ? 1 : 0, hd.mitschwingNoten.size());
        clip("zupf_Harfe_clip_DDur_mit", hd, {{0.0, ton(62, 0.5, 0.5)}, {0.2, ton(66, 0.5, 0.5)}, {0.4, ton(69, 0.5, 0.5)}}, 48000, true);
    }

    // die Bezuege
    int fehler = 0;
    for (const Fall& f : faelle)
    {
        const double ic = bezugMit(f.s, 0.5, 24000);
        if (hexVon(ic) != hexVon(f.s.ausgangBezug))
        {
            ++fehler;
            std::printf("# FEHLER: %s normiere hier anders als in der Vorgabe\n", f.name);
        }
        std::printf("bezug %s %s %.17g\n", f.name, hexVon(f.s.ausgangBezug).c_str(), f.s.ausgangBezug);
        const double lv = bezugMit(f.s, 1.0, 48000);
        std::printf("bezugLv %s %s %.17g\n", f.name, hexVon(lv).c_str(), lv);
    }

    // die gestimmten Laufzeitfaktoren
    for (const Fall& f : faelle) kurve(f.name, "laufzeit", f.s.laufzeitJeNote);
    std::printf("# normiere nachgerechnet: %s\n", fehler == 0 ? "gleich fuer alle fuenf" : "UNGLEICH");
    return fehler == 0 ? 0 : 1;
}
