// S192 vor dem Teilplan 34k: Messung an der IC-Vorlage der Klasse 2 (harvest/instruments-calculation), nichts im Repo
// geaendert. Muster: docs/modellierung/zunge/proben/2026-11-11_S188_ic_vorlage_messung.cpp. Bau: 2026-11-12_S192_bau.bat.
//
// A  Beleg des Baus: die drei IC-Abdruecke (werte/abdruck_msvc.txt: zupf_Gitarre_m52_mf, zupf_Cembalo_m60_mf,
//    zupf_Harfe_clip_C4_E4).
// B  je Instrument jede dritte Taste bei Staerke 0 / 0,5 / 1 (gezupft, 1 s gehalten, 2 s gerechnet): Tonhoehe zweifach
//    (die Spektralspitze der Vorlage ±60 Cent, unabhaengig davon YIN 0,1–0,5 s), Lautheit BS.1770 ueber den Ton, Spitze;
//    dazu bei 0,5 ohne Daempfen (laissez vibrer, 8 s) die Zeit bis −60 dB unter dem lautesten 20-ms-Block (T60) und die
//    Zeit bis −20 dB (frueher Abfall — zweistufig, wenn T20·3 deutlich unter T60 liegt).
// C  Spielweisen und Teile: Cembalo Lautenzug und Tastenstoss, Harfe laissez vibrer, Laute/Oud Doppelchor, Gitarre im
//    Clip mit den leeren Saiten, Rechenzeit.
#include "gemeinsam/Werkzeug.hpp"
#include "klassen/02_zupf_saite/Zupf.hpp"
#include "messung/Bausteine.hpp"

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

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

double yinCent(const std::vector<float>& t, int note)
{
    const std::size_t ab = kRate / 10, n = std::min<std::size_t>(t.size() - ab, static_cast<std::size_t>(0.4 * kRate));
    std::vector<double> x(t.begin() + static_cast<std::ptrdiff_t>(ab), t.begin() + static_cast<std::ptrdiff_t>(ab + n));
    const double f = hzVon(note);
    const messung::Tonhoehe th = messung::tonhoehe(x, kRate, std::max(20.0, f / 1.6), std::min(8000.0, f * 1.6));
    std::vector<double> hz;
    for (const double h : th.hz)
        if (h > 0.0) hz.push_back(h);
    if (hz.size() < 3) return 1e9;
    std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
    return 1200.0 * std::log2(hz[hz.size() / 2] / f);
}

bool endlich(const std::vector<float>& t)
{
    return std::all_of(t.begin(), t.end(), [](float v) { return std::isfinite(v); });
}

/// Zeit (s) ab dem lautesten 20-ms-Block bis zum letzten Block ueber `db` darunter.
double abfallS(const std::vector<float>& t, double db)
{
    const std::size_t b = kRate / 50;
    std::vector<double> e;
    for (std::size_t i = 0; i + b <= t.size(); i += b)
    {
        double s = 0.0;
        for (std::size_t k = 0; k < b; ++k) s += double(t[i + k]) * t[i + k];
        e.push_back(10.0 * std::log10(s / b + 1e-300));
    }
    if (e.empty()) return 0.0;
    const auto it = std::max_element(e.begin(), e.end());
    const std::size_t m = static_cast<std::size_t>(it - e.begin());
    std::size_t letzter = m;
    for (std::size_t i = m; i < e.size(); ++i)
        if (e[i] > *it + db) letzter = i;
    return (letzter - m) * 0.02;
}

struct Zeile
{
    double centSpitze = 0.0, centYin = 0.0, lufs = -300.0, spitze = 0.0;
    bool endlich = true;
};

Zeile messe(const ZupfSpiel& s, int note, double e)
{
    ZupfAnschlag a;
    a.note = note;
    a.staerke = e;
    a.haltenS = 1.0;
    std::vector<float> t;
    ZupfMessung m;
    rechneZupfTon(s, a, t, 2 * kRate, &m);
    Zeile z;
    z.endlich = endlich(t);
    z.centSpitze = m.schwingt ? 1200.0 * std::log2(m.spielHz / hzVon(note)) : 1e9;
    z.centYin = yinCent(t, note);
    z.lufs = lautheit(t, kRate);
    z.spitze = spitze(t);
    return z;
}

struct Ausklang
{
    double t20 = 0.0, t60 = 0.0;
};

Ausklang ausklang(const ZupfSpiel& s, int note)
{
    ZupfAnschlag a;
    a.note = note;
    a.haltenS = 8.0;
    a.laissezVibrer = true;
    std::vector<float> t;
    rechneZupfTon(s, a, t, 8 * kRate);
    return {abfallS(t, -20.0), abfallS(t, -60.0)};
}

}  // namespace

int main()
{
    // A: Beleg des Baus
    {
        const ZupfSpiel g = baueVorgabeGitarre();
        ZupfAnschlag a;
        a.note = 52;
        a.staerke = 0.5;
        a.haltenS = 0.8;
        std::vector<float> t;
        rechneZupfTon(g, a, t, 28800);
        std::printf("A Abdruck zupf_Gitarre_m52_mf %s\n", abdruck(t).c_str());
        const ZupfSpiel c = baueVorgabeCembalo();
        a.note = 60;
        rechneZupfTon(c, a, t, 28800);
        std::printf("A Abdruck zupf_Cembalo_m60_mf %s\n", abdruck(t).c_str());
        const ZupfSpiel h = baueVorgabeHarfe();
        ZupfAnschlag c4;
        c4.note = 60;
        c4.haltenS = 0.5;
        std::vector<float> h1;
        rechneZupfClip(h, {{0.0, c4}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, h1, 48000);
        std::printf("A Abdruck zupf_Harfe_clip_C4_E4 %s\n", abdruck(h1).c_str());
    }
    for (const ZupfSpiel& s : {baueVorgabeCembalo(), baueVorgabeHarfe(), baueVorgabeGitarre(), baueVorgabeLaute(), baueVorgabeOud()})
    {
        std::printf("\n## %s  Umfang %d-%d  Bezug %.6g  Chor %d (%.1f ct)  Stegmoden %zu  Guete %.0f  Kraft %.2f-%.2f N  Mitschwingen %zu\n",
                    s.name.c_str(), s.noteMin, s.noteMax, s.ausgangBezug, s.chor.saiten, s.chor.verstimmungCent, s.steg.size(), s.guete,
                    s.kraftPpN, s.kraftFfN, s.mitschwingNoten.size());
        std::printf("note\t| e0: ctSp ctYin LUFS | e.5: ctSp ctYin LUFS spitze | e1: ctSp ctYin LUFS | T20 T60 s\n");
        std::vector<int> noten;
        for (int n = s.noteMin; n <= s.noteMax; n += 3) noten.push_back(n);
        if (noten.back() != s.noteMax) noten.push_back(s.noteMax);
        std::vector<std::vector<Zeile>> alle(noten.size(), std::vector<Zeile>(3));
        std::vector<Ausklang> aus(noten.size());
        parallelJe(noten.size() * 4, [&](std::size_t i) {
            static const double e[3] = {0.0, 0.5, 1.0};
            if (i % 4 == 3)
                aus[i / 4] = ausklang(s, noten[i / 4]);
            else
                alle[i / 4][i % 4] = messe(s, noten[i / 4], e[i % 4]);
        }, 8);
        double minL = 1e9, maxL = -1e9, dMin = 1e9, dMax = -1e9;
        int daneben = 0, danebenYin = 0, unendlich = 0, ohneYin = 0;
        double t60min = 1e9, t60max = 0.0;
        for (std::size_t k = 0; k < noten.size(); ++k)
        {
            std::printf("%d", noten[k]);
            for (int j = 0; j < 3; ++j)
            {
                const Zeile& z = alle[k][static_cast<std::size_t>(j)];
                std::printf("\t| %+.1f %+.1f %.1f", z.centSpitze, z.centYin > 1e8 ? 0.0 : z.centYin, z.lufs);
                if (j == 1) std::printf(" %.3f", z.spitze);
                if (!z.endlich) ++unendlich;
                if (std::abs(z.centSpitze) > 10.0) ++daneben;
                if (z.centYin > 1e8)
                    ++ohneYin;
                else if (std::abs(z.centYin) > 10.0)
                    ++danebenYin;
            }
            std::printf("\t| %.2f %.2f\n", aus[k].t20, aus[k].t60);
            if (alle[k][1].lufs > -200.0)
            {
                minL = std::min(minL, alle[k][1].lufs);
                maxL = std::max(maxL, alle[k][1].lufs);
            }
            if (alle[k][0].lufs > -200.0 && alle[k][2].lufs > -200.0)
            {
                const double d = alle[k][2].lufs - alle[k][0].lufs;
                dMin = std::min(dMin, d);
                dMax = std::max(dMax, d);
            }
            t60min = std::min(t60min, aus[k].t60);
            t60max = std::max(t60max, aus[k].t60);
        }
        std::printf("Spanne LUFS ueber den Umfang bei 0,5: %.1f LU (%.1f … %.1f); Dynamik e0 -> e1 je Note %.1f bis %.1f LU\n", maxL - minL, minL, maxL,
                    dMin, dMax);
        std::printf("ueber 10 Cent daneben: Spitze %d, YIN %d (ohne YIN %d) von %zu; nicht endlich %d; T60 %.2f … %.2f s\n", daneben, danebenYin, ohneYin,
                    noten.size() * 3, unendlich, t60min, t60max);
    }
    // C: Spielweisen und Teile
    {
        const ZupfSpiel c = baueVorgabeCembalo();
        ZupfAnschlag a;
        a.note = 60;
        a.haltenS = 3.0;
        std::vector<float> n1, n2;
        rechneZupfTon(c, a, n1, 3 * kRate);
        a.lautenzug = true;
        rechneZupfTon(c, a, n2, 3 * kRate);
        std::printf("\nC Cembalo C4: T60 normal %.2f s, Lautenzug %.2f s; LUFS %.1f / %.1f\n", abfallS(n1, -60.0), abfallS(n2, -60.0), lautheit(n1, kRate),
                    lautheit(n2, kRate));
        ZupfSpiel ohne = c;
        ohne.tastenstoss = 0.0;
        for (const double e : {0.0, 1.0})
        {
            ZupfAnschlag b;
            b.note = 41;
            b.staerke = e;
            std::vector<float> mit, oh;
            rechneZupfTon(c, b, mit, kRate);
            rechneZupfTon(ohne, b, oh, kRate);
            std::printf("C Cembalo F2 Staerke %.0f: mit Tastenstoss %.1f LUFS, ohne %.1f LUFS\n", e, lautheit(mit, kRate), lautheit(oh, kRate));
        }
    }
    {
        const ZupfSpiel h = baueVorgabeHarfe();
        ZupfAnschlag a;
        a.note = 60;
        a.haltenS = 0.5;
        std::vector<float> d, l;
        rechneZupfTon(h, a, d, 4 * kRate);
        a.laissezVibrer = true;
        rechneZupfTon(h, a, l, 4 * kRate);
        std::printf("C Harfe C4: gedaempft nach 0,5 s T60 %.2f s, laissez vibrer %.2f s\n", abfallS(d, -60.0), abfallS(l, -60.0));
    }
    for (const ZupfSpiel& s : {baueVorgabeLaute(), baueVorgabeOud()})
    {
        ZupfSpiel einzeln = s;
        einzeln.chor = {1, 0.0};
        ZupfAnschlag a;
        a.note = 60;
        a.haltenS = 2.0;
        std::vector<float> z, e;
        rechneZupfTon(s, a, z, 2 * kRate);
        rechneZupfTon(einzeln, a, e, 2 * kRate);
        std::printf("C %s C4: Doppelchor %.1f LUFS, eine Saite %.1f LUFS, T60 %.2f / %.2f s\n", s.name.c_str(), lautheit(z, kRate), lautheit(e, kRate),
                    abfallS(z, -60.0), abfallS(e, -60.0));
    }
    {
        const ZupfSpiel g = baueVorgabeGitarre();
        ZupfAnschlag a;
        a.note = 57;
        a.haltenS = 0.6;
        std::vector<float> mit, ohne;
        rechneZupfClip(g, {{0.0, a}}, mit, 2 * kRate, true);
        rechneZupfClip(g, {{0.0, a}}, ohne, 2 * kRate, false);
        std::vector<float> spaet(mit.begin() + kRate, mit.end()), spaetO(ohne.begin() + kRate, ohne.end());
        std::printf("C Gitarre A3 im Clip, nach 1 s: mit leeren Saiten %.1f dB RMS, ohne %.1f dB\n", dB(rms(spaet)), dB(rms(spaetO)));
        std::vector<float> t;
        ZupfAnschlag b;
        b.note = 40;
        b.staerke = 1.0;
        b.haltenS = 2.0;
        double best = 1e9;
        for (int i = 0; i < 3; ++i)
        {
            const auto t0 = std::chrono::steady_clock::now();
            rechneZupfTon(g, b, t, 2 * kRate);
            best = std::min(best, std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
        }
        std::printf("C Rechenzeit Gitarre E2 je Note: %.2f %% der Echtzeit (unter Drosselung)\n", 100.0 * best / 2.0);
        const ZupfSpiel h = baueVorgabeHarfe();
        ZupfAnschlag c;
        c.note = 60;
        best = 1e9;
        for (int i = 0; i < 3; ++i)
        {
            const auto t0 = std::chrono::steady_clock::now();
            rechneZupfClip(h, {{0.0, c}}, t, 2 * kRate);
            best = std::min(best, std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
        }
        std::printf("C Rechenzeit Harfe im Clip (47 Saiten): %.2f %% der Echtzeit (unter Drosselung)\n", 100.0 * best / 2.0);
    }
    return 0;
}
