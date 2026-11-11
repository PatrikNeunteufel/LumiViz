// S188 Planung 34j: Messung an der IC-Vorlage der Zunge (harvest/instruments-calculation), nichts im Repo.
#include "gemeinsam/Werkzeug.hpp"
#include "klassen/10_zunge/Zunge.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace ic;
using namespace ic::zunge;

namespace
{

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

struct Zeile
{
    bool schwingt = false;
    double cent = 0.0, lufs = -300.0, rmsDb = -300.0, ein = 0.0, ausMs = 0.0, ampl = 0.0;
};

Zeile messe(const ZungenSpiel& s, int note, double e, const char** = nullptr)
{
    ZungenAnschlag a;
    a.note = note;
    a.expression = e;
    a.haltenS = 1.0;
    std::vector<float> t;
    ZungenMessung m;
    const std::int64_t frames = static_cast<std::int64_t>(2.0 * a.rate);
    rechneZungenTon(s, a, t, frames, &m);
    Zeile z;
    z.schwingt = m.schwingt;
    z.cent = m.schwingt ? 1200.0 * std::log2(m.spielHz / hzVon(note)) : 0.0;
    z.ein = m.einschwingS;
    z.ampl = m.amplitudeRel;
    std::vector<float> halt(t.begin(), t.begin() + 48000);
    z.lufs = lautheit(halt, a.rate);
    std::vector<float> mitte(t.begin() + 24000, t.begin() + 45600);
    z.rmsDb = dB(rms(mitte));
    // Ausklang: letzter Wert ueber −60 dB der Spitze des gehaltenen Teils, ab dem Ende der Note
    const double sp = spitze(mitte);
    std::size_t letzter = 48000;
    for (std::size_t i = 48000; i + 240 < t.size(); ++i)
        if (std::abs(t[i]) > 1e-3 * sp) letzter = i;
    z.ausMs = (static_cast<double>(letzter) - 48000.0) / 48.0;
    return z;
}

}  // namespace

int main()
{
    // A: der Abdruck des Akkordeons wie in IC test_zunge (Beleg, dass dieser Bau wie IC rechnet)
    {
        const ZungenSpiel s = baueVorgabeAkkordeon();
        ZungenAnschlag a;
        a.note = 69;
        a.expression = 0.5;
        a.haltenS = 0.8;
        std::vector<float> t;
        rechneZungenTon(s, a, t, 28800);
        std::printf("A Abdruck zunge_Akkordeon_m69_mf %s\n", abdruck(t).c_str());
    }
    for (const ZungenSpiel& s : {baueVorgabeAkkordeon(), baueVorgabeHarmonium(), baueVorgabeMelodica(), baueVorgabeMundharmonika()})
    {
        std::printf("\n## %s  Umfang %d-%d bassAb %d  Bezug %.6g  Choere %zu  Fensterpunkte %zu von %d\n", s.name.c_str(), s.noteMin, s.noteMax, s.bassAb,
                    s.ausgangBezug, s.choere.size(), s.druckUntenJeNote.punkte.size(), s.noteMax - s.noteMin + 1);
        std::printf("note\tunten\toben\t| e0: schw cent LUFS ein_ms | e.5: schw cent LUFS ein_ms | e1: schw cent LUFS ein_ms aus_ms ampl%%\n");
        std::vector<int> noten;
        for (int n = s.noteMin; n <= s.noteMax; n += 3) noten.push_back(n);
        if (noten.back() != s.noteMax) noten.push_back(s.noteMax);
        std::vector<std::vector<Zeile>> alle(noten.size(), std::vector<Zeile>(3));
        parallelJe(noten.size() * 3, [&](std::size_t i) {
            static const double e[3] = {0.0, 0.5, 1.0};
            alle[i / 3][i % 3] = messe(s, noten[i / 3], e[i % 3]);
        }, 8);
        double minL[3] = {1e9, 1e9, 1e9}, maxL[3] = {-1e9, -1e9, -1e9};
        double minLd[3] = {1e9, 1e9, 1e9}, maxLd[3] = {-1e9, -1e9, -1e9};
        int stumm = 0, daneben = 0;
        for (std::size_t k = 0; k < noten.size(); ++k)
        {
            const int n = noten[k];
            const int n0 = std::clamp(n, s.noteMin, s.noteMax);
            double u = 0.0, o = 0.0;
            for (const auto& p : s.druckUntenJeNote.punkte)
                if (p.note == n0) u = p.wert;
            for (const auto& p : s.druckObenJeNote.punkte)
                if (p.note == n0) o = p.wert;
            std::printf("%d\t%.0f\t%.0f", n, u, o);
            for (int j = 0; j < 3; ++j)
            {
                const Zeile& z = alle[k][static_cast<std::size_t>(j)];
                std::printf("\t| %d %+.1f %.1f %.0f", z.schwingt ? 1 : 0, z.cent, z.lufs, 1000.0 * z.ein);
                if (!z.schwingt) ++stumm;
                else if (std::abs(z.cent) > 10.0) ++daneben;
                const bool diskant = s.bassAb == 0 || n >= s.bassAb;
                if (z.lufs > -200.0)
                {
                    minL[j] = std::min(minL[j], z.lufs);
                    maxL[j] = std::max(maxL[j], z.lufs);
                    if (diskant)
                    {
                        minLd[j] = std::min(minLd[j], z.lufs);
                        maxLd[j] = std::max(maxLd[j], z.lufs);
                    }
                }
            }
            std::printf(" %.0f %.1f\n", alle[k][2].ausMs, 100.0 * alle[k][2].ampl);
        }
        std::printf("Spanne LUFS ueber den Umfang: e0 %.1f  e.5 %.1f  e1 %.1f LU", maxL[0] - minL[0], maxL[1] - minL[1], maxL[2] - minL[2]);
        if (s.bassAb > 0) std::printf("   nur Diskant: e0 %.1f  e.5 %.1f  e1 %.1f LU", maxLd[0] - minLd[0], maxLd[1] - minLd[1], maxLd[2] - minLd[2]);
        std::printf("\nohne Ton (YIN) %d, ueber 10 Cent daneben %d von %zu\n", stumm, daneben, noten.size() * 3);
        // Dynamik je Note e0 -> e1
        double dMin = 1e9, dMax = -1e9;
        for (std::size_t k = 0; k < noten.size(); ++k)
            if (alle[k][0].lufs > -200.0 && alle[k][2].lufs > -200.0)
            {
                const double d = alle[k][2].lufs - alle[k][0].lufs;
                dMin = std::min(dMin, d);
                dMax = std::max(dMax, d);
            }
        std::printf("Dynamik e0 -> e1 je Note: %.1f bis %.1f LU\n", dMin, dMax);
        // Spielweisen an der Mitte
        const int m = (std::max(s.noteMin, s.bassAb) + s.noteMax) / 2;
        for (const double bend : {-25.0, -50.0, -100.0})
        {
            ZungenAnschlag a, b;
            a.note = b.note = m;
            a.expression = b.expression = 0.6;
            a.haltenS = b.haltenS = 0.8;
            b.pitchCent = bend;
            ZungenMessung ma, mb;
            std::vector<float> ta, tb;
            rechneZungenTon(s, a, ta, 38400, &ma);
            rechneZungenTon(s, b, tb, 38400, &mb);
            std::printf("Bend %.0f Cent an %d: %+.1f Cent, Pegel %+.1f dB\n", bend, m, mb.schwingt && ma.schwingt ? 1200.0 * std::log2(mb.spielHz / ma.spielHz) : 0.0,
                        dB(rms(tb)) - dB(rms(ta)));
        }
    }
    return 0;
}
