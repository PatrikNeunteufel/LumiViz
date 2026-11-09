/**
 * @file test_trommel.cpp
 * @brief Klasse 5 Membran: ideales Fell (Bessel-Verhaeltnisse), Pauke gegen Packer 1993 (gemessene Verhaeltnisse),
 *        zwei Felle als Paare, Tabla harmonisch (Raman), Kontakt und Kraft (Dahl, Wagner), Gleiten (Marogna, Wagner),
 *        Pauke auf der Note, Spielweisen, Wirbel, Teppich, Determinismus, Abdruck, Rechenzeit
 */
#include "doctest.h"

#include "klassen/05_membran/Trommel.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <complex>
#include <cstdio>
#include <numbers>

using namespace ic;
using namespace ic::trommel;

namespace
{

constexpr double kPi = std::numbers::pi;

double modeHz(const std::vector<FellMode>& m, int mm, int n, int welche = 0)
{
    int k = 0;
    for (const FellMode& x : m)
        if (x.m == mm && x.n == n && k++ == welche) return x.hz;
    return 0.0;
}

const FellMode* mode(const std::vector<FellMode>& m, int mm, int n)
{
    for (const FellMode& x : m)
        if (x.m == mm && x.n == n) return &x;
    return nullptr;
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < t.size(); ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * double(t[ab + i]) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

const TrommelSpiel& pauke()
{
    static const TrommelSpiel p = baueVorgabePauke();
    return p;
}

}  // namespace

TEST_CASE("Membran: ohne Luft die Bessel-Verhaeltnisse des idealen Fells (j_11/j_01 = 1,593, j_21/j_01 = 2,136, j_02/j_01 = 2,295)")
{
    Fell f;
    f.abstrahlung = false;
    f.innenLuft = 0.0;
    const std::vector<FellMode> m = fellModen({f}, Kessel{}, 700.0);
    const double f01 = modeHz(m, 0, 1);
    REQUIRE(f01 > 0.0);
    CHECK(f01 == doctest::Approx(2.404826 / (2.0 * kPi * f.radiusM) * std::sqrt(f.spannungNm / f.dichteKgM2)).epsilon(1e-4));
    CHECK(modeHz(m, 1, 1) / f01 == doctest::Approx(1.5933).epsilon(1e-3));
    CHECK(modeHz(m, 2, 1) / f01 == doctest::Approx(2.1355).epsilon(1e-3));
    CHECK(modeHz(m, 0, 2) / f01 == doctest::Approx(2.2954).epsilon(1e-3));
}

TEST_CASE("Pauke gegen Packer 1993 Tab. 7.1 (gemessen, nur Bericht): (2,1) : (3,1) : (4,1) zu (1,1) = 1,50 : 1,97 : 2,44, (0,1) 0,81")
{
    const std::vector<FellMode>& m = *pauke().moden;
    const double f11 = modeHz(m, 1, 1);
    REQUIRE(f11 > 0.0);
    std::printf("Pauke: (1,1) %.0f Hz (Packer 172), (0,1) %.2f, (2,1) %.2f, (3,1) %.2f, (4,1) %.2f, (5,1) %.2f\n", f11, modeHz(m, 0, 1) / f11,
                modeHz(m, 2, 1) / f11, modeHz(m, 3, 1) / f11, modeHz(m, 4, 1) / f11, modeHz(m, 5, 1) / f11);
    CHECK(modeHz(m, 2, 1) / f11 == doctest::Approx(1.50).epsilon(0.04));
    CHECK(modeHz(m, 3, 1) / f11 == doctest::Approx(1.97).epsilon(0.04));
    CHECK(modeHz(m, 4, 1) / f11 == doctest::Approx(2.44).epsilon(0.04));
    CHECK(modeHz(m, 0, 1) / f11 == doctest::Approx(0.81).epsilon(0.08));
    // T60: (0,1) kurz (Packer < 0,3 s), (1,1) um 0,8 s, (2,1) laenger
    const FellMode* a = mode(m, 0, 1);
    const FellMode* b = mode(m, 1, 1);
    const FellMode* c = mode(m, 2, 1);
    REQUIRE((a && b && c));
    std::printf("Pauke T60: (0,1) %.2f s (Packer < 0,3), (1,1) %.2f s (0,8), (2,1) %.2f s (1,7)\n", a->t60S, b->t60S, c->t60S);
    CHECK(a->t60S < 0.3);
    CHECK(b->t60S > 0.3);
    CHECK(b->t60S < 1.5);
    CHECK(c->t60S > b->t60S);
}

TEST_CASE("Membran: zwei Felle ueber den Kessel — die (0,1) spaltet in ein Paar (Wagner 2006 an der Snare: 188 → 240/366 Hz, nur Bericht)")
{
    const TrommelSpiel tom = baueVorgabeTom();
    const double a = modeHz(*tom.moden, 0, 1, 0), b = modeHz(*tom.moden, 0, 1, 1);
    std::printf("Tom: (0,1)-Paar %.0f / %.0f Hz\n", a, b);
    REQUIRE(b > 0.0);
    CHECK(b / a > 1.3);
}

TEST_CASE("Tabla: der Belag macht die Moden harmonisch (Raman; Sathej & Adhikari — nur Bericht): (2,1), (0,2) ≈ 1,5; (3,1), (1,2) ≈ 2 × (1,1)")
{
    const TrommelSpiel t = baueVorgabeTabla();
    const std::vector<FellMode>& m = *t.moden;
    const double f11 = modeHz(m, 1, 1);
    std::printf("Tabla: (0,1) %.2f, (2,1) %.2f, (0,2) %.2f, (3,1) %.2f, (1,2) %.2f relativ zu (1,1)\n", modeHz(m, 0, 1) / f11, modeHz(m, 2, 1) / f11,
                modeHz(m, 0, 2) / f11, modeHz(m, 3, 1) / f11, modeHz(m, 1, 2) / f11);
    CHECK(modeHz(m, 2, 1) / f11 == doctest::Approx(1.5).epsilon(0.03));
    CHECK(modeHz(m, 0, 2) / f11 == doctest::Approx(1.5).epsilon(0.03));
    CHECK(modeHz(m, 3, 1) / f11 == doctest::Approx(2.0).epsilon(0.03));
    CHECK(modeHz(m, 1, 2) / f11 == doctest::Approx(2.0).epsilon(0.03));
}

TEST_CASE("Trommel: Kontakt und Kraft (Dahl: mf 5–8 ms, Wagner: 3 N … > 100 N), lauter mit der Staerke, Gleiten (Snare ff, Pauke klein)")
{
    const TrommelSpiel tom = baueVorgabeTom();
    double vorher = -1e9;
    for (const double e : {0.0, 0.5, 1.0})
    {
        TrommelAnschlag a;
        a.staerke = e;
        TrommelMessung m;
        std::vector<float> t;
        rechneTrommelTon(tom, a, t, 48000, &m);
        std::printf("Tom e%.1f: Kontakt %.1f ms, Kraft %.0f N, Gleiten %.1f Cent, %.1f dB\n", e, 1e3 * m.kontaktS, m.kraftSpitzeN, m.gleitenCent, dB(rms(t)));
        CHECK(m.kontaktS < 0.015);
        CHECK(m.kraftSpitzeN > 3.0);
        CHECK(m.kraftSpitzeN < 300.0);
        CHECK(dB(rms(t)) > vorher);
        vorher = dB(rms(t));
    }
    TrommelAnschlag ff;
    ff.staerke = 1.0;
    TrommelMessung ms, mp;
    std::vector<float> t;
    rechneTrommelTon(baueVorgabeSnare(), ff, t, 48000, &ms);
    ff.note = 52;
    rechneTrommelTon(pauke(), ff, t, 48000, &mp);
    std::printf("Gleiten ff: Snare %.1f Cent (Wagner ≈ 37), Pauke %.1f Cent („a minor effect in timpani“)\n", ms.gleitenCent, mp.gleitenCent);
    CHECK(ms.gleitenCent > 2.0);
    CHECK(ms.gleitenCent < 100.0);
    CHECK(mp.gleitenCent < ms.gleitenCent);
}

TEST_CASE("Pauke: die Note waehlt die Spannung — (1,1) auf der Note, Glissando ueber das Pedal")
{
    for (const int note : {48, 52, 55})
    {
        TrommelAnschlag a;
        a.note = note;
        std::vector<float> t;
        rechneTrommelTon(pauke(), a, t, 48000);
        const double f = 440.0 * std::exp2((note - 69) / 12.0);
        CAPTURE(note);
        // die (1,1) auf der Note: das Band dort staerker als 1 Halbton daneben
        CHECK(band(t, 2400, 16384, f) > 3.0 * band(t, 2400, 16384, f * std::exp2(1.0 / 12.0)));
    }
    TrommelAnschlag g;
    g.note = 48;
    g.glissandoCent = 400.0;
    std::vector<float> a, b;
    rechneTrommelTon(pauke(), g, a, 48000);
    g.glissandoCent = 0.0;
    rechneTrommelTon(pauke(), g, b, 48000);
    CHECK(a != b);
}

TEST_CASE("Trommel: Spielweisen, Wirbel, Teppich, alle Instrumente klingen")
{
    const TrommelSpiel snare = baueVorgabeSnare();
    std::vector<float> normal;
    rechneTrommelTon(snare, TrommelAnschlag{}, normal, 24000);
    for (const Schlag s : {Schlag::Mitte, Schlag::Rand, Schlag::Randschlag, Schlag::Reifen, Schlag::Gedaempft})
    {
        TrommelAnschlag a;
        a.schlag = s;
        std::vector<float> t;
        rechneTrommelTon(snare, a, t, 24000);
        CHECK(t != normal);
        CHECK(rms(t) > 0.0);
    }
    TrommelAnschlag w;
    w.wirbelS = 0.8;
    std::vector<float> wirbel;
    rechneTrommelTon(snare, w, wirbel, 48000);
    // der Wirbel klingt nach 0,6 s noch (ein Schlag allein ist dann weit abgeklungen)
    double spaet = 0.0;
    for (std::size_t i = 28800; i < 33600; ++i) spaet += double(wirbel[i]) * wirbel[i];
    CHECK(spaet > 0.0);
    // der Teppich: mehr Energie 3–8 kHz als ohne
    TrommelAnschlag o;
    o.ohneTeppich = true;
    std::vector<float> ohne;
    rechneTrommelTon(snare, o, ohne, 24000);
    double em = 0.0, eo = 0.0;
    for (double f = 3000.0; f <= 8000.0; f += 100.0)
    {
        em += std::pow(band(normal, 0, 8192, f), 2);
        eo += std::pow(band(ohne, 0, 8192, f), 2);
    }
    std::printf("Snare: 3–8 kHz mit Teppich %+.1f dB gegen ohne\n", 10.0 * std::log10(em / eo));
    CHECK(em > eo);
    for (const TrommelSpiel& s : {baueVorgabeBassdrum(), baueVorgabeConga(), baueVorgabeBongo()})
    {
        CAPTURE(s.name);
        std::vector<float> t;
        rechneTrommelTon(s, TrommelAnschlag{}, t, 24000);
        CHECK(rms(t) > 0.0);
    }
}

TEST_CASE("Trommel: Determinismus, Abdruck, Rechenzeit")
{
    const TrommelSpiel tom = baueVorgabeTom();
    std::vector<float> a;
    rechneTrommelTon(tom, TrommelAnschlag{}, a, 24000);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneTrommelTon(tom, TrommelAnschlag{}, q[i], 24000); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
    const std::string ist = abdruck(a);
    CHECK(test::erwarteterAbdruck("trommel_Tom_mf", ist) == ist);
    TrommelAnschlag p;
    p.note = 52;
    std::vector<float> b;
    rechneTrommelTon(pauke(), p, b, 24000);
    const std::string istP = abdruck(b);
    CHECK(test::erwarteterAbdruck("trommel_Pauke_m52_mf", istP) == istP);
    for (const TrommelSpiel* s : {&tom, &pauke()})
    {
        std::vector<float> t;
        TrommelAnschlag x;
        x.staerke = 1.0;
        x.note = s->note;
        const double anteil = echtzeitAnteil([&] { rechneTrommelTon(*s, x, t, 96000); }, 2.0);
        std::printf("Rechenzeit %s ff: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", s->name.c_str(), 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
        CHECK(anteil < test::echtzeitGrenzeHier());
    }
}
