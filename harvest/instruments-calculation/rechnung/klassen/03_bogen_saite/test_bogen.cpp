/**
 * @file test_bogen.cpp
 * @brief Klasse 3 Bogen-Saite: Pizzicato harmonisch, Helmholtz-Bewegung (Haften 1 − β), Schelleng-Fenster, jede dritte
 *        Note bei pp/mf/ff, Einschwingen (Guettler & Askenfelt 1997), Wolfton am Cello, Vibrato mit AM von selbst,
 *        Dämpfer (Elie et al.), Spielweisen, Ensemble, Viola und Kontrabass, Determinismus, Abdruck, Rechenzeit
 */
#include "doctest.h"

#include "klassen/03_bogen_saite/Bogen.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <complex>
#include <cstdio>
#include <numbers>

using namespace ic;
using namespace ic::bogen;

namespace
{

constexpr double kPi = std::numbers::pi;

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

StreichAnschlag ton(int note, double e, double haltenS = 0.8)
{
    StreichAnschlag a;
    a.kette = {{0.0, note, hzVon(note)}};
    a.expression = e;
    a.haltenS = haltenS;
    return a;
}

const StreichSpiel& cello()
{
    static const StreichSpiel c = baueVorgabeCello();
    return c;
}

const StreichSpiel& violine()
{
    static const StreichSpiel v = baueVorgabeVioline();
    return v;
}

const StreichSpiel& viola()
{
    static const StreichSpiel v = baueVorgabeViola();
    return v;
}

const StreichSpiel& kontrabass()
{
    static const StreichSpiel k = baueVorgabeKontrabass();
    return k;
}

double band(const std::vector<float>& t, std::size_t ab, std::size_t n, double f)
{
    std::complex<double> s(0.0, 0.0);
    for (std::size_t i = 0; i < n; ++i)
        s += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * double(t[ab + i]) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * (ab + i) / 48000.0));
    return std::abs(s);
}

}  // namespace

TEST_CASE("Bogen: Pizzicato der leeren D-Saite des Cellos ist harmonisch (Waveguide ohne Steifigkeit)")
{
    StreichAnschlag a = ton(50, 0.6, 1.0);
    a.art = Art::Pizzicato;
    std::vector<float> t;
    rechneStreichTon(cello(), a, t, 48000);
    const double f = hzVon(50);
    // die Spitze je Teilton nahe k·f (±1 %): suchen im Band
    for (int k = 1; k <= 4; ++k)
    {
        double best = 0.0, beste = 0.0;
        for (double x = 0.99 * k * f; x <= 1.01 * k * f; x += 0.1)
        {
            const double b = band(t, 4800, 32768, x);
            if (b > best)
            {
                best = b;
                beste = x;
            }
        }
        CAPTURE(k);
        CHECK(beste / (k * f) == doctest::Approx(1.0).epsilon(0.006));
    }
}

TEST_CASE("Bogen: Helmholtz-Bewegung am Cello D3 — Haften ≈ 1 − β je Periode, raues Kratzen weit ueber dem Fenster")
{
    const StreichSpiel& c = cello();
    StreichMessung m;
    std::vector<float> t;
    rechneStreichTon(c, ton(50, 0.5), t, 38400, &m);
    REQUIRE(m.schwingt);
    const double beta = c.bogenAbstandM / (c.mensurM * 1.0);
    std::printf("Cello D3 mf: Haften %.2f (1 − β = %.2f), %+.1f Cent\n", m.haftAnteil, 1.0 - beta, 1200.0 * std::log2(m.spielHz / hzVon(50)));
    CHECK(std::abs(m.haftAnteil - (1.0 - beta)) < 0.25);
    // das Fenster: unten < oben, und Galluzzo 2017 S. 8 spielte D3 bei β = 0,08 im Bereich 0,4–3,2 N (nur Bericht):
    // das Fenster des Modells liegt in derselben Groessenordnung
    const double u = wertAn(c.kraftUntenJeNote, 50, Skala::Logarithmisch), o = wertAn(c.kraftObenJeNote, 50, Skala::Logarithmisch);
    const double uf = wertAn(c.kraftUntenFfJeNote, 50, Skala::Logarithmisch), of = wertAn(c.kraftObenFfJeNote, 50, Skala::Logarithmisch);
    std::printf("Cello D3: Fenster der Bogenkraft %.3f … %.3f N bei v_B pp, %.3f … %.3f N bei ff\n", u, o, uf, of);
    // Schelleng: F_max ∝ v_B — das Fenster bei ff reicht hoeher
    CHECK(of > o);
    CHECK(o > u);
    CHECK(o > 0.1);
    CHECK(o < 10.0);
    StreichAnschlag rau = ton(50, 0.5);
    rau.kraftN = 8.0 * o;
    StreichMessung mr;
    rechneStreichTon(c, rau, t, 38400, &mr);
    CHECK((!mr.schwingt || std::abs(1200.0 * std::log2(mr.spielHz / hzVon(50))) > 50.0 || mr.haftAnteil > 0.97));
}

TEST_CASE("Bogen: jede dritte Note bei pp, mf, ff — alle vier mindestens 90 % auf ±10 Cent, ff lauter als pp")
{
    for (const StreichSpiel* s : {&cello(), &violine(), &viola(), &kontrabass()})
    {
        int getroffen = 0, gesamt = 0;
        for (int note = s->noteMin; note <= s->noteMax; note += 3)
        {
            double pp = 0.0;
            for (const double e : {0.0, 0.5, 1.0})
            {
                StreichMessung m;
                std::vector<float> t;
                rechneStreichTon(*s, ton(note, e), t, 38400, &m);
                ++gesamt;
                const double c = m.schwingt ? 1200.0 * std::log2(m.spielHz / hzVon(note)) : 1e9;
                if (std::abs(c) < 10.0)
                    ++getroffen;
                else
                    std::printf("WARN %s n%d e%.1f: %+.0f Cent\n", s->name.c_str(), note, e, c);
                const double pegel = dB(rms(t));
                if (e == 0.0) pp = pegel;
                if (e == 1.0) CHECK(pegel > pp);
            }
        }
        std::printf("%s: %d von %d Pruefpunkten auf ±10 Cent\n", s->name.c_str(), getroffen, gesamt);
        CHECK(getroffen * 10 >= gesamt * 9);
    }
}

TEST_CASE("Bogen: Einschwingen am Cello unter 90 ms (Guettler & Askenfelt 1997: annehmbar 50 ms, mit Mehrfachgleiten 90 ms)")
{
    for (const int note : {43, 50, 57})
    {
        StreichMessung m;
        std::vector<float> t;
        rechneStreichTon(cello(), ton(note, 0.5), t, 38400, &m);
        CAPTURE(note);
        std::printf("Cello n%d: Einschwingen %.0f ms\n", note, 1000.0 * m.einschwingS);
        CHECK(m.einschwingS < 0.09);
    }
}

TEST_CASE("Bogen: Wolfton — an der Korpusmode (F3 ≈ 174 Hz) braucht das Cello mehr Kraft als eine Quarte tiefer")
{
    const StreichSpiel& c = cello();
    const double wolf = wertAn(c.kraftUntenJeNote, 53, Skala::Logarithmisch);
    const double ruhig = wertAn(c.kraftUntenJeNote, 48, Skala::Logarithmisch);
    std::printf("Cello: kleinste Kraft F3 (Wolf) %.3f N, C3 %.3f N\n", wolf, ruhig);
    // Woodhouse 2014 (nur Bericht): die Korpusresonanz hebt die Mindestkraft
    CHECK(wolf > ruhig);
}

TEST_CASE("Bogen: Vibrato (FM) bringt die Amplitudenmodulation je Teilton von selbst (F-5 a); Daempfer daempft 1–2,5 kHz (Elie et al.)")
{
    const StreichSpiel& v = violine();
    StreichAnschlag a = ton(69, 0.5, 1.6);
    std::vector<float> ohne, mit;
    rechneStreichTon(v, a, ohne, 76800);
    a.vibratoCent = 25.0;
    rechneStreichTon(v, a, mit, 76800);
    // die Huelle des staerksten der Teiltoene 1–3 (Fenster 1024: die Hauptkeule ±47 Hz ist breiter als der Hub ±0,7 %
    // = ±10 Hz am dritten Teilton; schwache Teiltoene an einer Antiresonanz des Korpus schwanken auch ohne Vibrato)
    const auto am = [&](const std::vector<float>& t) {
        double bestMittel = -1.0, schwankung = 0.0;
        for (int k = 1; k <= 3; ++k)
        {
            double lo = 1e300, hi = 0.0, summe = 0.0;
            int zahl = 0;
            for (std::size_t i = 24000; i + 1024 <= 72000; i += 480)
            {
                const double b = band(t, i, 1024, k * hzVon(69));
                lo = std::min(lo, b);
                hi = std::max(hi, b);
                summe += b;
                ++zahl;
            }
            if (summe / zahl > bestMittel)
            {
                bestMittel = summe / zahl;
                schwankung = dB(hi / std::max(lo, 1e-300));
            }
        }
        return schwankung;
    };
    const double hOhne = am(ohne), hMit = am(mit);
    std::printf("Violine A4: Huelle der Teiltoene ohne Vibrato %.1f dB, mit %.1f dB\n", hOhne, hMit);
    CHECK(hMit > hOhne + 1.0);
    // der Daempfer am Pizzicato (ohne Reibregime — wie Elies Messung der Stegmobilitaet)
    StreichAnschlag d = ton(69, 0.6, 1.0);
    d.art = Art::Pizzicato;
    std::vector<float> offen, sord;
    rechneStreichTon(v, d, offen, 48000);
    d.daempfer = true;
    rechneStreichTon(v, d, sord, 48000);
    double eo = 0.0, es = 0.0;
    for (double f = 1000.0; f <= 2500.0; f += 25.0)
    {
        eo += band(offen, 480, 16384, f) * band(offen, 480, 16384, f);
        es += band(sord, 480, 16384, f) * band(sord, 480, 16384, f);
    }
    std::printf("Violine A4 pizz. con sordino: 1–2,5 kHz %+.1f dB (Elie: 5–13 dB weniger)\n", 10.0 * std::log10(es / eo));
    CHECK(es < eo);
}

TEST_CASE("Bogen: Spielweisen unterscheiden sich, Ensemble aus Stimmen, Viola und Kontrabass ungemessen gekennzeichnet")
{
    const StreichSpiel& c = cello();
    std::vector<float> arco;
    rechneStreichTon(c, ton(50, 0.5), arco, 38400);
    for (const Art art : {Art::Pizzicato, Art::Spiccato, Art::Tremolo, Art::ColLegno})
    {
        StreichAnschlag a = ton(50, 0.5);
        a.art = art;
        std::vector<float> t;
        rechneStreichTon(c, a, t, 38400);
        CHECK(t != arco);
        CHECK(rms(t) > 0.0);
    }
    for (const Lage l : {Lage::Ponticello, Lage::Tasto})
    {
        StreichAnschlag a = ton(50, 0.5);
        a.lage = l;
        std::vector<float> t;
        rechneStreichTon(c, a, t, 38400);
        CHECK(t != arco);
    }
    std::vector<float> e1, e2;
    rechneEnsembleTon(violine(), ton(69, 0.5), 8, e1, 38400);
    rechneEnsembleTon(violine(), ton(69, 0.5), 8, e2, 38400);
    CHECK(e1 == e2);
    std::vector<float> eins;
    rechneStreichTon(violine(), ton(69, 0.5), eins, 38400);
    CHECK(e1 != eins);
    for (const StreichSpiel* s : {&viola(), &kontrabass()})
    {
        CAPTURE(s->name);
        CHECK(s->spielbar);
        CHECK(s->kennzeichen.find("ungemessen") != std::string::npos);
    }
}

TEST_CASE("Bogen: Determinismus, Abdruck, Rechenzeit")
{
    const StreichSpiel& c = cello();
    std::vector<float> a;
    rechneStreichTon(c, ton(50, 0.5), a, 28800);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneStreichTon(c, ton(50, 0.5), q[i], 28800); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
    const std::string ist = abdruck(a);
    CHECK(test::erwarteterAbdruck("bogen_Cello_m50_mf", ist) == ist);
    std::vector<float> v;
    rechneStreichTon(violine(), ton(76, 0.5), v, 28800);
    const std::string istV = abdruck(v);
    CHECK(test::erwarteterAbdruck("bogen_Violine_m76_mf", istV) == istV);
    std::vector<float> va, kb;
    rechneStreichTon(viola(), ton(62, 0.5), va, 28800);
    rechneStreichTon(kontrabass(), ton(40, 0.5), kb, 28800);
    const std::string istVa = abdruck(va), istKb = abdruck(kb);
    CHECK(test::erwarteterAbdruck("bogen_Viola_m62_mf", istVa) == istVa);
    CHECK(test::erwarteterAbdruck("bogen_Kontrabass_m40_mf", istKb) == istKb);
    std::vector<float> t;
    const double anteil = echtzeitAnteil([&] { rechneStreichTon(c, ton(c.noteMin, 1.0, 2.0), t, 96000); }, 2.0);
    std::printf("Rechenzeit Cello C2 ff: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
    CHECK(anteil < test::echtzeitGrenzeHier());
}
