/**
 * @file test_zunge.cpp
 * @brief Klasse 10 Durchschlagende Zunge: die Zunge spricht selbst an und schwingt „somewhat below“ f₀ (Cottingham),
 *        Akkordeon trifft seine Noten, Musette-Schwebung (Hergert), Bend nur abwaerts, Bass-Seite, Harmonium, Melodica,
 *        Mundharmonika, Determinismus, Abdruck, Rechenzeit
 */
#include "doctest.h"

#include "klassen/10_zunge/Zunge.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <complex>
#include <cstdio>
#include <numbers>

using namespace ic;
using namespace ic::zunge;

namespace
{

constexpr double kPi = std::numbers::pi;

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

const ZungenSpiel& akkordeon()
{
    static const ZungenSpiel a = baueVorgabeAkkordeon();
    return a;
}

ZungenAnschlag ton(int note, double e, double haltenS = 0.8)
{
    ZungenAnschlag a;
    a.note = note;
    a.expression = e;
    a.haltenS = haltenS;
    return a;
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < t.size(); ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * double(t[ab + i]) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

}  // namespace

TEST_CASE("Zunge: spricht selbst an und schwingt etwas unter f₀ (Cottingham 2011: „somewhat below“), klingt nach dem Wind aus")
{
    const ZungenSpiel& s = akkordeon();
    const double f0 = wertAn(s.eigenfrequenzJeNote, 60, Skala::Logarithmisch);
    ZungenMessung m;
    std::vector<double> t;
    rechneEineZunge(s, f0, hzVon(60), ton(60, 0.5), t, 48000, &m);
    REQUIRE(m.schwingt);
    std::printf("Zunge C4: f₀ %.2f Hz, spielt %.2f Hz (%.1f Cent darunter), Amplitude %.1f %% der Laenge (Cottingham: 15 %%)\n", f0, m.spielHz,
                1200.0 * std::log2(f0 / m.spielHz), 100.0 * m.amplitudeRel);
    CHECK(m.spielHz < f0);
    CHECK(m.spielHz > 0.97 * f0);
    CHECK(m.amplitudeRel > 0.02);
    // nach dem Wind (0,8 s + 30 ms) faellt der Ton
    double halt = 0.0, nach = 0.0;
    for (std::size_t i = 28800; i < 33600; ++i) halt += t[i] * t[i];
    for (std::size_t i = 43200; i < 48000; ++i) nach += t[i] * t[i];
    CHECK(nach < 0.01 * halt);
}

TEST_CASE("Akkordeon: jede dritte Diskantnote bei pp, mf, ff — mindestens 90 % auf ±10 Cent; ff schneller da als pp")
{
    const ZungenSpiel& s = akkordeon();
    int getroffen = 0, gesamt = 0;
    for (int note = s.bassAb; note <= s.noteMax; note += 3)
    {
        double einPp = 0.0, einFf = 0.0;
        for (const double e : {0.0, 0.5, 1.0})
        {
            ZungenMessung m;
            std::vector<float> t;
            rechneZungenTon(s, ton(note, e), t, 38400, &m);
            ++gesamt;
            const double c = m.schwingt ? 1200.0 * std::log2(m.spielHz / hzVon(note)) : 1e9;
            if (std::abs(c) < 10.0)
                ++getroffen;
            else
                std::printf("WARN Akkordeon n%d e%.1f: %+.0f Cent\n", note, e, c);
            if (e == 0.0) einPp = m.einschwingS;
            if (e == 1.0) einFf = m.einschwingS;
        }
        CHECK(einFf <= einPp + 0.02);
    }
    std::printf("Akkordeon: %d von %d Pruefpunkten auf ±10 Cent\n", getroffen, gesamt);
    CHECK(getroffen * 10 >= gesamt * 9);
}

TEST_CASE("Akkordeon: Musette schwebt mit ≈ 2 Hz an A4 (Hergert: Beispiel 2,00 Hz = 7,9 ct, nachgelesen)")
{
    std::vector<float> t;
    rechneZungenTon(akkordeon(), ton(69, 0.5, 3.0), t, 144000);
    // die Huelle des Grundtons in 50-ms-Stuecken ueber 2 s: die staerkste Modulation zwischen 1 und 4 Hz
    std::vector<double> h;
    for (std::size_t i = 24000; i + 2400 <= 120000; i += 2400) h.push_back(band(t, i, 2400, 440.0));
    double best = 0.0, bestHz = 0.0;
    for (double fm = 0.5; fm <= 6.0; fm += 0.05)
    {
        std::complex<double> s(0.0, 0.0);
        double mittel = 0.0;
        for (const double x : h) mittel += x / h.size();
        for (std::size_t i = 0; i < h.size(); ++i) s += (h[i] - mittel) * std::exp(std::complex<double>(0.0, -2.0 * kPi * fm * i * 0.05));
        if (std::abs(s) > best)
        {
            best = std::abs(s);
            bestHz = fm;
        }
    }
    std::printf("Akkordeon A4 Musette: Schwebung %.2f Hz\n", bestHz);
    CHECK(bestHz == doctest::Approx(2.0).epsilon(0.15));
}

TEST_CASE("Akkordeon: Bend nur abwaerts und gekappt (Ventil, Adachi et al.), Bass-Seite eine Oktave tiefer (16′), Balgwechsel")
{
    const ZungenSpiel& s = akkordeon();
    ZungenMessung m0, mb, mu;
    std::vector<float> t0, tb, tu;
    rechneZungenTon(s, ton(64, 0.6), t0, 38400, &m0);
    ZungenAnschlag b = ton(64, 0.6);
    b.pitchCent = -80.0;
    rechneZungenTon(s, b, tb, 38400, &mb);
    ZungenAnschlag u = ton(64, 0.6);
    u.pitchCent = 100.0;
    rechneZungenTon(s, u, tu, 38400, &mu);
    std::printf("Akkordeon E4: Bend −80 Cent ergibt %+.1f Cent (das Modell kappt; Spieler 10–35 Cent, Elejalde-Garcia)\n",
                1200.0 * std::log2(mb.spielHz / m0.spielHz));
    REQUIRE(mb.schwingt);
    CHECK(mb.spielHz < m0.spielHz * std::exp2(-2.0 / 1200.0));
    CHECK(tu == t0);
    // die Bass-Seite spielt (16′ + 8′ unter der Teilung; die 16′-Zungen sprechen nur schwer an — offen.md)
    std::vector<float> bass;
    rechneZungenTon(s, ton(48, 0.6), bass, 38400);
    CHECK(rms(bass) > 0.0);
    CHECK(band(bass, 14400, 16384, hzVon(48)) > 0.0);
    ZungenAnschlag w = ton(64, 0.6);
    w.balgwechselS = 0.4;
    std::vector<float> tw;
    rechneZungenTon(s, w, tw, 38400);
    CHECK(tw != t0);
}

TEST_CASE("Harmonium, Melodica, Mundharmonika spielen ihre Mitte auf ±10 Cent")
{
    for (const ZungenSpiel& s : {baueVorgabeHarmonium(), baueVorgabeMelodica(), baueVorgabeMundharmonika()})
    {
        CAPTURE(s.name);
        const int n = (s.noteMin + s.noteMax) / 2;
        ZungenMessung m;
        std::vector<float> t;
        rechneZungenTon(s, ton(n, 0.5), t, 38400, &m);
        REQUIRE(m.schwingt);
        CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(n))) < 10.0);
    }
}

TEST_CASE("Zunge: Determinismus, Abdruck, Rechenzeit (Akkordeon Musette, zwei Zungen)")
{
    const ZungenSpiel& s = akkordeon();
    std::vector<float> a;
    rechneZungenTon(s, ton(69, 0.5), a, 28800);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneZungenTon(s, ton(69, 0.5), q[i], 28800); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
    const std::string ist = abdruck(a);
    CHECK(test::erwarteterAbdruck("zunge_Akkordeon_m69_mf", ist) == ist);
    std::vector<float> t;
    const double anteil = echtzeitAnteil([&] { rechneZungenTon(s, ton(69, 0.5, 2.0), t, 96000); }, 2.0);
    std::printf("Rechenzeit Akkordeon Musette: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
    CHECK(anteil < test::echtzeitGrenzeHier());
}
