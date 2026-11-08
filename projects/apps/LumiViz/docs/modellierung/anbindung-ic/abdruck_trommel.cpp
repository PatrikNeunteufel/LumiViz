/**
 * @file abdruck_trommel.cpp
 * @brief Beleg 34h-2 (S179): die Abdruecke der Klasse 5 aus den IC-Quellen, ohne Aenderung am IC-Repo
 *
 * IC fuehrt fuer die Klasse 5 nur zwei Abdruecke (`trommel_Tom_mf`, `trommel_Pauke_m52_mf` in `werte/abdruck_msvc.txt`).
 * Dieses Programm rechnet mit den IC-Quellen (`bb7e6d6`) unter MSVC (`/MD`, die Plattform von `abdruck_msvc.txt`) die uebrigen, die LumiViz Bit fuer Bit
 * nachbilden muss (Instrumente_IC_Konzept.md 34h-2, TM1 a, TM2 a). Es gehoert zu keinem Bau; der Baubefehl steht in
 * `abdruck_trommel.bat` daneben, die Ausgabe ist `tests/unit/UnitTests/werte/trommel_ic.txt`.
 *
 * Inhalt:
 * - die zwei IC-Abdruecke (mf, `staerke` 0,5) als Gegenprobe zu `abdruck_msvc.txt`;
 * - je Instrument ff (`staerke` 1) an der Mitte seines Umfangs — der Weg des Adapters bei Stufe 0 (TM1 a:
 *   tempo = tempoFf · 10^(0/20), dieselbe Zahl wie tempoPp · (tempoFf/tempoPp)^1 bei den sieben Vorgaben);
 * - die Spielweisen der Snare bei ff (TM2 a: `stroke`, `roll`, `snaresOff` am Beginn der Note);
 * - je Instrument der Bezug `ausgangBezug` aus IC `normiere` (0,5 s) und mit der Probe von LumiViz (1 s) — die Spitze liegt
 *   am Anfang, beide muessen gleich sein.
 */
#include "klassen/05_membran/Trommel.hpp"
#include "gemeinsam/Werkzeug.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace ic;
using namespace ic::trommel;

namespace
{

/// IC `normiere` (klassen/05_membran/Vorgaben.cpp:25–40) mit einer waehlbaren Laenge der Probe.
double bezugMit(TrommelSpiel s, std::int64_t frames)
{
    s.ausgangBezug = 1.0;
    bereite(s);
    for (int runde = 0; runde < 2; ++runde)
    {
        TrommelAnschlag a;
        a.staerke = 1.0;
        a.note = (s.note + s.noteMax) / 2;
        std::vector<float> ton;
        rechneTrommelTon(s, a, ton, frames);
        const double sp = spitze(ton) * s.ausgangBezug;
        s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
    }
    return s.ausgangBezug;
}

std::string hexVon(double x)
{
    std::uint64_t b = 0;
    std::memcpy(&b, &x, sizeof b);
    char t[32];
    std::snprintf(t, sizeof t, "%016llx", static_cast<unsigned long long>(b));
    return t;
}

void zeile(const char* name, const TrommelSpiel& s, const TrommelAnschlag& a, std::int64_t frames)
{
    std::vector<float> t;
    rechneTrommelTon(s, a, t, frames);
    std::printf("abdruck %s %s\n", name, abdruck(t).c_str());
}

}  // namespace

int main()
{
    std::printf("# Bezuege der Klasse 5 aus InstrumentsCalculation (bb7e6d6), gerechnet 34h-2 (S179) mit\n");
    std::printf("# docs/modellierung/anbindung-ic/abdruck_trommel.cpp unter MSVC (/MD) — nicht von Hand aendern\n");
    std::printf("# abdruck <name> <sha256>: 24000 Frames bei 48 kHz (Wirbel 48000); mf = staerke 0,5, ff = staerke 1\n");
    std::printf("# bezug <name> <hex der 64 Bit> <Zahl>: ausgangBezug nach IC normiere (zwei Runden)\n");

    struct Fall
    {
        const char* name;
        TrommelSpiel s;
    };
    const std::vector<Fall> faelle = {{"Tom", baueVorgabeTom()},           {"Pauke", baueVorgabePauke()}, {"Bassdrum", baueVorgabeBassdrum()},
                                      {"Snare", baueVorgabeSnare()},       {"Tabla", baueVorgabeTabla()}, {"Conga", baueVorgabeConga()},
                                      {"Bongo", baueVorgabeBongo()}};

    // die zwei IC-Abdruecke (test_trommel.cpp:213–229)
    zeile("trommel_Tom_mf", faelle[0].s, TrommelAnschlag{}, 24000);
    {
        TrommelAnschlag p;
        p.note = 52;
        zeile("trommel_Pauke_m52_mf", faelle[1].s, p, 24000);
    }
    // ff an der Mitte des Umfangs
    for (const Fall& f : faelle)
    {
        TrommelAnschlag a;
        a.staerke = 1.0;
        a.note = (f.s.note + f.s.noteMax) / 2;
        zeile(("trommel_" + std::string(f.name) + "_ff").c_str(), f.s, a, 24000);
    }
    // die Spielweisen der Snare bei ff
    const TrommelSpiel& snare = faelle[3].s;
    const struct
    {
        const char* name;
        Schlag schlag;
    } schlaege[] = {{"mitte", Schlag::Mitte}, {"rand", Schlag::Rand}, {"randschlag", Schlag::Randschlag}, {"reifen", Schlag::Reifen},
                    {"gedaempft", Schlag::Gedaempft}};
    for (const auto& x : schlaege)
    {
        TrommelAnschlag a;
        a.staerke = 1.0;
        a.note = snare.note;
        a.schlag = x.schlag;
        zeile(("trommel_Snare_" + std::string(x.name) + "_ff").c_str(), snare, a, 24000);
    }
    {
        TrommelAnschlag a;
        a.staerke = 1.0;
        a.note = snare.note;
        a.ohneTeppich = true;
        zeile("trommel_Snare_ohneTeppich_ff", snare, a, 24000);
        a.ohneTeppich = false;
        a.wirbelS = 0.8;
        zeile("trommel_Snare_wirbel_ff", snare, a, 48000);
    }
    // die Bezuege: IC (0,5 s) und die Probe von LumiViz (1 s)
    int ungleich = 0;
    for (const Fall& f : faelle)
    {
        const double ic = bezugMit(f.s, 24000);
        const double lv = bezugMit(f.s, 48000);
        if (hexVon(ic) != hexVon(f.s.ausgangBezug)) std::printf("# FEHLER: %s normiere hier anders als in der Vorgabe\n", f.name);
        if (hexVon(ic) != hexVon(lv))
        {
            ++ungleich;
            std::printf("# UNGLEICH: %s mit 1 s %s %.17g\n", f.name, hexVon(lv).c_str(), lv);
        }
        std::printf("bezug %s %s %.17g\n", f.name, hexVon(ic).c_str(), ic);
    }
    std::printf("# Probe 1 s gleich 0,5 s: %s\n", ungleich == 0 ? "ja, alle sieben" : "NEIN");
    return 0;
}
