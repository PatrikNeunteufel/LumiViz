/**
 * @file abdruck_zunge.cpp
 * @brief Beleg 34j-1 (S189): die Abdruecke der Klasse 10 aus der IC-Vorlage, ohne Aenderung an der Vorlage
 *
 * IC fuehrt fuer die Klasse 10 nur einen Abdruck (`zunge_Akkordeon_m69_mf` in `werte/abdruck_msvc.txt`). Dieses Programm
 * rechnet mit der Vorlage unter `harvest/instruments-calculation/rechnung` (IC `4409088`) unter MSVC (`/MD`, die
 * Plattform von `abdruck_msvc.txt`) die uebrigen, die LumiViz in 34j-2 und 34j-3 Bit fuer Bit nachbilden muss
 * (Instrumente_IC_Konzept.md, Teilplan 34j, ZV1 a). Es gehoert zu keinem Bau; der Baubefehl steht in `abdruck_zunge.bat`
 * daneben, die Ausgabe ist `tests/unit/UnitTests/werte/zunge_ic.txt`.
 *
 * Inhalt:
 * - der IC-Abdruck (m69, Expression 0,5, 0,8 s gehalten, 28 800 Frames) als Gegenprobe zu `abdruck_msvc.txt`;
 * - je Instrument tief / mitte / hoch des Umfangs (am Akkordeon der Diskant ab `bassAb`) bei Expression 0 / 0,5 / 1,
 *   0,4 s gehalten, 28 800 Frames (mit dem Absetzen des Winds und dem Ausklang);
 * - am Akkordeon die Bass-Seite tief / mitte / hoch bei Expression 0,5;
 * - je Instrument an der Mitte Vibrato (20 Cent, 5,5 Hz) und Bend (−50 Cent), am Akkordeon der Balgwechsel bei 0,2 s;
 * - je Instrument der Bezug `ausgangBezug` aus IC `normiere` (0,6 s bei Expression 1) und zum Vergleich mit einer
 *   Probe von 1 s (die Laenge der Probe von LumiViz) — beim gehaltenen Ton sind sie nicht von selbst gleich;
 * - je Instrument die drei Kurven aus `Stimmung.inc` (f₀ und das Fenster des Drucks je Note) Punkt fuer Punkt in Hex.
 */
#include "klassen/10_zunge/Zunge.hpp"
#include "gemeinsam/Werkzeug.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace ic;
using namespace ic::zunge;

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

/// IC `normiere` (klassen/10_zunge/Vorgaben.cpp:27–39) mit waehlbarer Laenge der Probe.
double bezugMit(ZungenSpiel s, double sekunden)
{
    s.ausgangBezug = 1.0;
    ZungenAnschlag a;
    a.note = (std::max(s.noteMin, s.bassAb) + s.noteMax) / 2;
    a.expression = 1.0;
    a.haltenS = sekunden;
    std::vector<float> ton;
    rechneZungenTon(s, a, ton, static_cast<std::int64_t>(sekunden * a.rate));
    const double sp = spitze(ton);
    return sp > 0.0 ? sp / 0.5 : 1.0;
}

ZungenAnschlag ton(int note, double e, double haltenS = 0.4)
{
    ZungenAnschlag a;
    a.note = note;
    a.expression = e;
    a.haltenS = haltenS;
    return a;
}

void zeile(const std::string& name, const ZungenSpiel& s, const ZungenAnschlag& a, std::int64_t frames = kFrames)
{
    std::vector<float> t;
    rechneZungenTon(s, a, t, frames);
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
    std::printf("# Bezuege der Klasse 10 aus InstrumentsCalculation (4409088, Vorlage unter harvest/), gerechnet 34j-1 (S189)\n");
    std::printf("# mit docs/modellierung/anbindung-ic/abdruck_zunge.cpp unter MSVC (/MD) — nicht von Hand aendern\n");
    std::printf("# abdruck <name> <sha256>: 28800 Frames bei 48 kHz; _e0/_e05/_e1 = Expression; 0,4 s gehalten, ausser\n");
    std::printf("#   zunge_Akkordeon_m69_mf (IC test_zunge: Expression 0,5, 0,8 s gehalten)\n");
    std::printf("# bezug <name> <hex der 64 Bit> <Zahl>: ausgangBezug nach IC normiere (0,6 s, Expression 1, Mitte ab bassAb)\n");
    std::printf("# bezug1s <name> <hex> <Zahl>: dieselbe Rechnung mit einer Probe von 1 s\n");
    std::printf("# kurve <name> <eigenfrequenz|druckUnten|druckOben> <note> <hex> <Zahl>: die Stimmung aus Stimmung.inc\n");

    struct Fall
    {
        const char* name;
        ZungenSpiel s;
    };
    const std::vector<Fall> faelle = {{"Akkordeon", baueVorgabeAkkordeon()},
                                      {"Harmonium", baueVorgabeHarmonium()},
                                      {"Melodica", baueVorgabeMelodica()},
                                      {"Mundharmonika", baueVorgabeMundharmonika()}};

    // der IC-Abdruck (test_zunge.cpp:170–179)
    zeile("zunge_Akkordeon_m69_mf", faelle[0].s, ton(69, 0.5, 0.8));

    // tief / mitte / hoch bei Expression 0 / 0,5 / 1
    const struct
    {
        const char* name;
        double e;
    } stufen[] = {{"e0", 0.0}, {"e05", 0.5}, {"e1", 1.0}};
    for (const Fall& f : faelle)
    {
        const int tief = std::max(f.s.noteMin, f.s.bassAb);
        const int noten[] = {tief, (tief + f.s.noteMax) / 2, f.s.noteMax};
        for (const int n : noten)
            for (const auto& st : stufen)
                zeile("zunge_" + std::string(f.name) + "_m" + std::to_string(n) + "_" + st.name, f.s, ton(n, st.e));
    }

    // die Bass-Seite des Akkordeons
    {
        const ZungenSpiel& s = faelle[0].s;
        const int noten[] = {s.noteMin, (s.noteMin + s.bassAb - 1) / 2, s.bassAb - 1};
        for (const int n : noten) zeile("zunge_Akkordeon_bass_m" + std::to_string(n) + "_e05", s, ton(n, 0.5));
    }

    // Vibrato, Bend, Balgwechsel an der Mitte
    for (const Fall& f : faelle)
    {
        const int mitte = (std::max(f.s.noteMin, f.s.bassAb) + f.s.noteMax) / 2;
        const std::string basis = "zunge_" + std::string(f.name) + "_m" + std::to_string(mitte);
        ZungenAnschlag v = ton(mitte, 0.5);
        v.vibratoCent = 20.0;
        v.vibratoHz = 5.5;
        zeile(basis + "_vibrato", f.s, v);
        ZungenAnschlag b = ton(mitte, 0.5);
        b.pitchCent = -50.0;
        zeile(basis + "_bend", f.s, b);
    }
    {
        const ZungenSpiel& s = faelle[0].s;
        ZungenAnschlag w = ton(69, 0.5);
        w.balgwechselS = 0.2;
        zeile("zunge_Akkordeon_m69_balgwechsel", s, w);
    }

    // die Bezuege
    int fehler = 0;
    for (const Fall& f : faelle)
    {
        const double ic = bezugMit(f.s, 0.6);
        if (hexVon(ic) != hexVon(f.s.ausgangBezug))
        {
            ++fehler;
            std::printf("# FEHLER: %s normiere hier anders als in der Vorgabe\n", f.name);
        }
        std::printf("bezug %s %s %.17g\n", f.name, hexVon(f.s.ausgangBezug).c_str(), f.s.ausgangBezug);
        const double lv = bezugMit(f.s, 1.0);
        std::printf("bezug1s %s %s %.17g\n", f.name, hexVon(lv).c_str(), lv);
    }

    // die Kurven
    for (const Fall& f : faelle)
    {
        kurve(f.name, "eigenfrequenz", f.s.eigenfrequenzJeNote);
        kurve(f.name, "druckUnten", f.s.druckUntenJeNote);
        kurve(f.name, "druckOben", f.s.druckObenJeNote);
    }
    std::printf("# normiere nachgerechnet: %s\n", fehler == 0 ? "gleich fuer alle vier" : "UNGLEICH");
    return fehler == 0 ? 0 : 1;
}
