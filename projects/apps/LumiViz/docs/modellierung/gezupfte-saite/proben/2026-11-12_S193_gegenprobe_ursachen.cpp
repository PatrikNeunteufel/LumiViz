// S193 vor dem Teilplan 34k: Gegenprobe an der IC-Vorlage der Klasse 2 (harvest/instruments-calculation) — woran haengt
// das Gefaelle der Lautheit ueber den Umfang (Befund S192: 18,6–36,1 LU)? Nichts im Repo geaendert, die Vorlage wird nur
// mit anderen Werten gefahren. Bau: 2026-11-12_S192_bau.bat <diese Datei> <ziel.exe>.
//
// Zwei Vermutungen aus dem Lesen von Zupf.cpp / Vorgaben.cpp:
//   U1  Der Zupf ist ein Kraftstoss fester Dauer (stossS 2–4 ms, Hann): sein Spektrum hat die erste Nullstelle bei
//       2 / stossS (500–1000 Hz) — Grundtoene darueber werden kaum angeregt (Abfall im Diskant).
//   U2  Der Steg traegt 5–10 einzelne Moden hoher Guete: liegt ein Grundton auf einer Mode, springt die Lautheit.
//
// Varianten je Instrument (jede dritte Taste, Staerke 0,5, 1 s gehalten, 2 s gerechnet, Lautheit BS.1770 wie S192):
//   V0  Vorlage unveraendert
//   V1  Stossdauer = 1/4 Periode der Note (statt fest)               -> prueft U1
//   V2  Steg dicht: 6 Moden je Oktave von der tiefsten Mode der Vorlage bis 6 kHz, Guete 8, gleiche Spitze
//       (Mittel der Vorlage), Richtungen abwechselnd — ein glatter Steg, KEIN Modell eines Instruments -> prueft U2
//   V3  V1 und V2 zusammen
// Ausgabe je Variante: Spanne ueber den Umfang, groesster Sprung zwischen gemessenen Nachbarn, die Reihe der LUFS.
#include "gemeinsam/Werkzeug.hpp"
#include "klassen/02_zupf_saite/Zupf.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace ic;
using namespace ic::zupf;

namespace
{

constexpr int kRate = 48000;

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

ZupfSpiel mitDichtemSteg(const ZupfSpiel& s)
{
    ZupfSpiel d = s;
    double tief = 1e9, summe = 0.0, winkel = 0.0;
    for (const StegMode& m : s.steg)
    {
        tief = std::min(tief, m.hz);
        summe += m.spitze;
        winkel = std::max(winkel, std::abs(m.winkelGrad));
    }
    const double spitze = summe / static_cast<double>(s.steg.size());
    d.steg.clear();
    int i = 0;
    for (double hz = tief; hz <= 6000.0; hz *= std::exp2(1.0 / 6.0))
        d.steg.push_back({hz, 8.0, spitze, (i++ % 2 == 0 ? 1.0 : -1.0) * winkel});
    stimme(d);
    return d;
}

double lufsVon(const ZupfSpiel& s, int note, bool stossMitPeriode)
{
    ZupfSpiel u = s;
    if (stossMitPeriode) u.stossS = 0.25 / hzVon(note);
    ZupfAnschlag a;
    a.note = note;
    a.staerke = 0.5;
    a.haltenS = 1.0;
    std::vector<float> t;
    rechneZupfTon(u, a, t, 2 * kRate);
    return lautheit(t, kRate);
}

void reihe(const char* name, const ZupfSpiel& s, const std::vector<int>& noten, bool stossMitPeriode)
{
    std::vector<double> l(noten.size(), -300.0);
    parallelJe(noten.size(), [&](std::size_t i) { l[i] = lufsVon(s, noten[i], stossMitPeriode); }, 8);
    double lo = 1e9, hi = -1e9, sprung = 0.0;
    int sprungBei = 0;
    for (std::size_t i = 0; i < l.size(); ++i)
    {
        lo = std::min(lo, l[i]);
        hi = std::max(hi, l[i]);
        if (i > 0 && std::abs(l[i] - l[i - 1]) > sprung)
        {
            sprung = std::abs(l[i] - l[i - 1]);
            sprungBei = noten[i];
        }
    }
    std::printf("%s  Spanne %.1f LU (%.1f … %.1f)  groesster Sprung %.1f LU (vor Taste %d)\n   ", name, hi - lo, lo, hi, sprung, sprungBei);
    for (std::size_t i = 0; i < l.size(); ++i) std::printf(" %d:%.1f", noten[i], l[i]);
    std::printf("\n");
}

}  // namespace

int main()
{
    for (const ZupfSpiel& s : {baueVorgabeCembalo(), baueVorgabeHarfe(), baueVorgabeGitarre(), baueVorgabeLaute(), baueVorgabeOud()})
    {
        std::vector<int> noten;
        for (int n = s.noteMin; n <= s.noteMax; n += 3) noten.push_back(n);
        if (noten.back() != s.noteMax) noten.push_back(s.noteMax);
        std::printf("\n## %s  Umfang %d-%d  stossS %.4f s (Nullstelle %.0f Hz)  Stegmoden %zu\n", s.name.c_str(), s.noteMin, s.noteMax, s.stossS,
                    2.0 / s.stossS, s.steg.size());
        const ZupfSpiel dicht = mitDichtemSteg(s);
        reihe("V0 Vorlage            ", s, noten, false);
        reihe("V1 Stoss 1/4 Periode  ", s, noten, true);
        reihe("V2 Steg dicht         ", dicht, noten, false);
        reihe("V3 beides             ", dicht, noten, true);
    }
    return 0;
}
