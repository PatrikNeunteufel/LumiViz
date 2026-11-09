/**
 * @file test_zupf.cpp
 * @brief Klasse 2 Gezupfte Saite: Tonhoehe ueber den Umfang, zwei Polarisationen an der 2×2-Admittanz (zweistufiges
 *        Abklingen, Bank & Karjalainen), Zupfpunkt des Cembalos (Valimaki), Lautenzug, Tastenstoss (Penttinen), Harfe
 *        laissez vibrer, Doppelchor der Laute, Koto zurueckgestellt, Determinismus, Abdruck, Rechenzeit
 */
#include "doctest.h"

#include "klassen/02_zupf_saite/Zupf.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdio>
#include <numbers>

using namespace ic;
using namespace ic::zupf;

namespace
{

constexpr double kPi = std::numbers::pi;

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < t.size(); ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * double(t[ab + i]) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

const ZupfSpiel& cembalo()
{
    static const ZupfSpiel c = baueVorgabeCembalo();
    return c;
}

ZupfAnschlag ton(int note, double e = 0.5, double haltenS = 0.8)
{
    ZupfAnschlag a;
    a.note = note;
    a.staerke = e;
    a.haltenS = haltenS;
    return a;
}

}  // namespace

TEST_CASE("Zupf: jede fuenfte Note aller Instrumente auf ±5 Cent (Spitze des Grundtons)")
{
    for (const ZupfSpiel& s : {cembalo(), baueVorgabeHarfe(), baueVorgabeGitarre(), baueVorgabeLaute(), baueVorgabeOud()})
    {
        for (int note = s.noteMin; note <= s.noteMax; note += 5)
        {
            CAPTURE(s.name);
            CAPTURE(note);
            ZupfMessung m;
            std::vector<float> t;
            rechneZupfTon(s, ton(note), t, 28800, &m);
            REQUIRE(m.schwingt);
            CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(note))) < 5.0);
        }
    }
}

TEST_CASE("Zupf: zwei Polarisationen am Steg — mit Kopplung zweistufiges Abklingen (Bank & Karjalainen 2010, nachgelesen PDF-S. 5)")
{
    // die Gitarre am Ton nahe der staerksten Stegmode (195 Hz ≈ G3): Abklingrate frueh (0,1–0,6 s) gegen spaet (1,5–2,5 s)
    const ZupfSpiel g = baueVorgabeGitarre();
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
    std::printf("Gitarre G3: Abklingrate frueh/spaet — eine Polarisation %.2f, zwei gekoppelt %.2f\n", einfach, zweifach);
    // ohne Kopplung bleibt ein Rest (das Einschwingen im fruehen Fenster); mit Kopplung deutlich zweistufig
    CHECK(zweifach > 2.0 * einfach);
}

TEST_CASE("Cembalo: Zupfpunkt 12 % im Bass, 50 % im Diskant (Valimaki et al. 2004) — oben fehlen die geraden Teiltoene")
{
    std::vector<float> diskant, bass;
    rechneZupfTon(cembalo(), ton(89), diskant, 28800);
    rechneZupfTon(cembalo(), ton(31), bass, 28800);
    const double d = dB(band(diskant, 2400, 16384, 2.0 * hzVon(89)) / band(diskant, 2400, 16384, hzVon(89)));
    const double b = dB(band(bass, 2400, 16384, 2.0 * hzVon(31)) / band(bass, 2400, 16384, hzVon(31)));
    std::printf("Cembalo: H2/H1 Diskant f³ %.1f dB, Bass G1 %.1f dB\n", d, b);
    CHECK(d < b - 10.0);
}

TEST_CASE("Cembalo: Lautenzug kuerzer, Tastenstoss lauter mit der Staerke (Penttinen: +10 dB unter 300 Hz je Stufe)")
{
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
    std::printf("Cembalo: unter 300 Hz ff gegen pp %+.1f dB\n", 10.0 * std::log10(ef / ep));
    CHECK(ef > ep);
}

TEST_CASE("Harfe: laissez vibrer klingt weiter, sonst daempft die Hand (F-7 a); Laute: Doppelchor schwebt; Koto zurueckgestellt")
{
    const ZupfSpiel h = baueVorgabeHarfe();
    std::vector<float> gedaempft, frei;
    rechneZupfTon(h, ton(60, 0.5, 0.5), gedaempft, 48000);
    ZupfAnschlag lv = ton(60, 0.5, 0.5);
    lv.laissezVibrer = true;
    rechneZupfTon(h, lv, frei, 48000);
    CHECK(band(gedaempft, 36000, 8192, hzVon(60)) < 0.1 * band(frei, 36000, 8192, hzVon(60)));
    const ZupfSpiel l = baueVorgabeLaute();
    std::vector<float> t;
    rechneZupfTon(l, ton(60, 0.6, 3.0), t, 144000);
    CHECK(t.size() == 144000u);
    const ZupfSpiel k = baueVorgabeKoto();
    CHECK_FALSE(k.spielbar);
    std::vector<float> stumm;
    rechneZupfTon(k, ton(60), stumm, 4800);
    CHECK(spitze(stumm) == 0.0);
}

TEST_CASE("Zupf je Clip (K2 F-3 a): eine Saite wie je Note; die leeren Saiten der Gitarre schwingen mit; Harfe mit 47 Saiten")
{
    const ZupfSpiel g = baueVorgabeGitarre();
    ZupfAnschlag a;
    a.note = 57;  // A3
    a.staerke = 0.7;
    a.haltenS = 0.3;
    // ohne Mitschwingen ist der Clip mit einer Note der Ton je Note (bis auf die Rundung)
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
    // mit Mitschwingen: nach dem Daempfen klingt 220 Hz weiter (2. Teilton der leeren A-Saite) — und 110 Hz kommt dazu
    std::vector<float> mit;
    rechneZupfClip(g, {{0.0, a}}, mit, 72000, true);
    std::vector<float> ohne;
    rechneZupfClip(g, {{0.0, a}}, ohne, 72000, false);
    const double m220 = band(mit, 33600, 24000, 220.0), o220 = band(ohne, 33600, 24000, 220.0);
    std::printf("Gitarre A3 gedaempft, 0,7–1,2 s bei 220 Hz: mit leeren Saiten %+.1f dB gegen ohne\n", 20.0 * std::log10(m220 / std::max(1e-30, o220)));
    CHECK(m220 > 10.0 * o220);
    // Harfe: 47 Saiten am Steg, deterministisch
    const ZupfSpiel h = baueVorgabeHarfe();
    CHECK(h.mitschwingNoten.size() == 47u);
    ZupfAnschlag c;
    c.note = 60;
    c.haltenS = 0.5;
    std::vector<float> h1, h2;
    rechneZupfClip(h, {{0.0, c}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, h1, 48000);
    rechneZupfClip(h, {{0.0, c}, {0.25, ZupfAnschlag{64, 0.5, 0.5}}}, h2, 48000);
    CHECK(h1 == h2);
    CHECK(spitze(h1) > 0.0);
    const std::string ist = abdruck(h1);
    CHECK(test::erwarteterAbdruck("zupf_Harfe_clip_C4_E4", ist) == ist);
}

TEST_CASE("Harfe: Pedale (K2 F-6 a) — Diagramm, Vorschlag, Bisbigliando, Glissando, Spur, nicht spielbar")
{
    const ZupfSpiel h = baueVorgabeHarfe();
    const Pedale cDur{0, 0, 0, 0, 0, 0, 0};
    CHECK(harfenSaiten(h, cDur) == h.mitschwingNoten);
    // D-Dur: F♯ und C♯
    bool ok = false;
    const Pedale d = pedaleVorschlag({62, 64, 66, 67, 69, 71, 73}, &ok);
    CHECK(ok);
    CHECK(d == Pedale{1, 0, 0, 1, 0, 0, 0});
    // E♯ und F♮ geben beide F: zwei Saiten — Bisbigliando
    const Pedale eis{0, 0, 1, 0, 0, 0, 0};
    CHECK(saitenFuer(65, eis).size() == 2u);
    CHECK_FALSE(spielbar(64, eis));  // E klingt nicht mehr
    // Glissando C4 → C5 in C-Dur: acht Saiten; abwaerts umgekehrt
    const std::vector<int> g = glissandoNoten(h, 60, 72, cDur);
    CHECK(g == std::vector<int>{60, 62, 64, 65, 67, 69, 71, 72});
    CHECK(glissandoNoten(h, 72, 60, cDur).front() == 72);
    // acht verschiedene Tonklassen auf einmal gehen nicht (sieben Saiten je Oktave)
    (void)pedaleVorschlag({60, 61, 62, 63, 64, 65, 66, 67}, &ok);
    CHECK_FALSE(ok);
    // nacheinander chromatisch: die Spur wechselt Pedale, jede Note ist unter ihrer Stelle spielbar
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
    // die Harfe unter D-Dur: die mitschwingenden Saiten tragen F♯ und C♯
    const ZupfSpiel hd = harfeMitPedalen(h, d);
    CHECK(std::find(hd.mitschwingNoten.begin(), hd.mitschwingNoten.end(), 66) != hd.mitschwingNoten.end());
    CHECK(std::find(hd.mitschwingNoten.begin(), hd.mitschwingNoten.end(), 65) == hd.mitschwingNoten.end());
}

TEST_CASE("Zupf: Determinismus, Abdruck, Rechenzeit")
{
    const ZupfSpiel g = baueVorgabeGitarre();
    std::vector<float> a;
    rechneZupfTon(g, ton(52), a, 28800);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneZupfTon(g, ton(52), q[i], 28800); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
    const std::string ist = abdruck(a);
    CHECK(test::erwarteterAbdruck("zupf_Gitarre_m52_mf", ist) == ist);
    std::vector<float> c;
    rechneZupfTon(cembalo(), ton(60), c, 28800);
    const std::string istC = abdruck(c);
    CHECK(test::erwarteterAbdruck("zupf_Cembalo_m60_mf", istC) == istC);
    std::vector<float> t;
    const double anteil = echtzeitAnteil([&] { rechneZupfTon(g, ton(40, 1.0, 2.0), t, 96000); }, 2.0);
    std::printf("Rechenzeit Gitarre E2: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
    CHECK(anteil < test::echtzeitGrenzeHier());
}
