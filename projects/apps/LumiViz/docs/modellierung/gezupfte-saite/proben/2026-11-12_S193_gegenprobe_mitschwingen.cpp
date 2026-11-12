// S193 vor dem Teilplan 34k: Gegenprobe zum Mitschwingen an der IC-Vorlage der Klasse 2 — was kostet es je Note, und wie
// viel traegt eine Auswahl der Saiten? Nichts im Repo geaendert. Bau: 2026-11-12_S192_bau.bat <diese Datei> <ziel.exe>.
//
// Gedanke: Saiten und Steg der Vorlage sind linear. Das Mitschwingen einer ungespielten Saite ist dann die Antwort auf
// EINE gespielte Note und laesst sich je Note rechnen (die gespielte Saite und die mitschwingenden an einem Steg), ohne
// den Rechenweg Clip — solange die mitschwingenden Saiten frei bleiben. Die Kosten wachsen mit der Zahl der Saiten.
//
// A  Gitarre: eine Note mit den sechs leeren Saiten (rechneZupfClip mit einer Note) — Rechenzeit und der Nachklang
//    (RMS 1–2 s nach dem Zupfen, die gespielte Saite nach 0,5 s gedaempft) gegen die Note ohne Mitschwingen.
// B  Harfe: dieselbe Messung an C4, G2, E5 mit (1) allen 47 Saiten, (2) nur den Saiten im Abstand von Oktave, Quinte,
//    Quarte zur Note (Teiltoene 2:1, 3:2, 4:3, 3:1, 4:1 …), (3) nur den Oktaven, (4) ohne. Rechenzeit je Fall.
#include "gemeinsam/Werkzeug.hpp"
#include "klassen/02_zupf_saite/Zupf.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace ic;
using namespace ic::zupf;

namespace
{

constexpr int kRate = 48000;

struct Ergebnis
{
    double nachklangDb = -300.0, lufs = -300.0, zeitProzent = 0.0;
    std::size_t saiten = 0;
};

Ergebnis messe(const ZupfSpiel& s, int note, bool mitschwingen)
{
    ZupfAnschlag a;
    a.note = note;
    a.staerke = 0.5;
    a.haltenS = 0.5;
    std::vector<float> t;
    double best = 1e9;
    for (int i = 0; i < 3; ++i)
    {
        const auto t0 = std::chrono::steady_clock::now();
        rechneZupfClip(s, {{0.0, a}}, t, 2 * kRate, mitschwingen);
        best = std::min(best, std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
    }
    Ergebnis e;
    const std::vector<float> spaet(t.begin() + kRate, t.end());
    e.nachklangDb = dB(rms(spaet));
    e.lufs = lautheit(t, kRate);
    e.zeitProzent = 100.0 * best / 2.0;
    e.saiten = mitschwingen ? s.mitschwingNoten.size() : 0;
    return e;
}

void zeile(const char* was, const Ergebnis& e)
{
    std::printf("  %-34s Saiten %2zu  Nachklang 1-2 s %7.1f dB RMS  Lautheit %6.1f LUFS  Rechenzeit %5.2f %%\n", was, e.saiten, e.nachklangDb, e.lufs,
                e.zeitProzent);
}

}  // namespace

int main()
{
    {
        const ZupfSpiel g = baueVorgabeGitarre();
        for (const int note : {45, 57, 64, 76})
        {
            std::printf("A Gitarre Note %d\n", note);
            zeile("ohne Mitschwingen", messe(g, note, false));
            zeile("mit den sechs leeren Saiten", messe(g, note, true));
        }
    }
    {
        const ZupfSpiel h = baueVorgabeHarfe();
        for (const int note : {43, 60, 76})
        {
            std::printf("B Harfe Note %d\n", note);
            zeile("ohne Mitschwingen", messe(h, note, false));
            zeile("alle Saiten", messe(h, note, true));
            for (int stufe = 0; stufe < 3; ++stufe)
            {
                ZupfSpiel u = h;
                u.mitschwingNoten.clear();
                for (const int n : h.mitschwingNoten)
                {
                    const int d = ((n - note) % 12 + 12) % 12;
                    const bool oktave = d == 0, quinte = d == 7 || d == 5, terz = d == 4 || d == 9 || d == 3 || d == 8;
                    if (n == note) continue;
                    if (oktave || (stufe >= 1 && quinte) || (stufe >= 2 && terz)) u.mitschwingNoten.push_back(n);
                }
                static const char* name[3] = {"nur Oktaven", "Oktaven, Quinten, Quarten", "dazu Terzen und Sexten"};
                zeile(name[stufe], messe(u, note, true));
            }
        }
    }
    // C  Linearitaet: ein Clip mit zwei Noten (C4 bei 0, E4 bei 0,25 s) gegen die Summe zweier Clips mit je einer Note,
    //    alle 47 Saiten am Steg. (1) beide laissez vibrer — keine Saite wird gedaempft; (2) beide nach 0,5 s gedaempft.
    {
        const ZupfSpiel h = baueVorgabeHarfe();
        for (const bool frei : {true, false})
        {
            ZupfAnschlag c4, e4;
            c4.note = 60;
            e4.note = 64;
            c4.haltenS = e4.haltenS = 0.5;
            c4.laissezVibrer = e4.laissezVibrer = frei;
            std::vector<float> beide, a, b;
            rechneZupfClip(h, {{0.0, c4}, {0.25, e4}}, beide, 2 * kRate);
            rechneZupfClip(h, {{0.0, c4}}, a, 2 * kRate);
            rechneZupfClip(h, {{0.25, e4}}, b, 2 * kRate);
            std::vector<float> diff(beide.size());
            for (std::size_t i = 0; i < beide.size(); ++i) diff[i] = beide[i] - (a[i] + b[i]);
            std::printf("C Harfe C4 + E4 %s: Clip %.1f dB RMS, Clip minus Summe der Einzelnoten %.1f dB RMS\n",
                        frei ? "laissez vibrer" : "nach 0,5 s gedaempft", dB(rms(beide)), dB(rms(diff)));
        }
    }
    return 0;
}
