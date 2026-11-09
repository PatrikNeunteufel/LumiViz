/**
 * @file test_gemeinsam.cpp
 * @brief Tests der Bausteine aller Klassen (AUFTRAG_BAU §3): Kurven, Eigenloeser mit Entartung, FFT, Faltung,
 *        SHA-256, K-Filter, WAV, modaler Kern, Stoss (Hertz), Balken, Luftraum, Rohr, Pool
 */
#include "doctest.h"

#include "gemeinsam/Balken.hpp"
#include "gemeinsam/Eigenzerlegung.hpp"
#include "gemeinsam/Fft.hpp"
#include "gemeinsam/Koerper.hpp"
#include "gemeinsam/Kurve.hpp"
#include "gemeinsam/Modal.hpp"
#include "gemeinsam/Rohr.hpp"
#include "gemeinsam/Werkzeug.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <numbers>

using namespace ic;

TEST_CASE("Kurve: Randwerte, Abweichung, monoton zwischen den Punkten, Klemmen")
{
    Kurve k = kurveAusPunkten({{40, 1.0}, {60, 2.0}, {80, 8.0}}, Belegkraft::Quelle, "Test");
    CHECK(wertAn(k, 20, Skala::Linear) == 1.0);
    CHECK(wertAn(k, 100, Skala::Linear) == 8.0);
    CHECK(wertAn(k, 60, Skala::Linear) == doctest::Approx(2.0));
    double vorher = 0.0;
    for (int n = 40; n <= 80; ++n)
    {
        const double w = wertAn(k, n, Skala::Logarithmisch);
        CHECK(w >= vorher);
        vorher = w;
    }
    // logarithmisch zwischen 2 und 8 in der Mitte: das geometrische Mittel liegt nahe 4
    CHECK(wertAn(kurveAusPunkten({{60, 2.0}, {80, 8.0}}), 70, Skala::Logarithmisch) == doctest::Approx(4.0));
    k.abweichungen.push_back({70, 100.0});
    CHECK(wertAn(k, 70, Skala::Linear) == 100.0);
    Kurve ersatz = kurveKonstant(5.0, Belegkraft::Offen, "Ersatz");
    Kurve leer;
    richteKurve(leer, 0.0, 10.0, ersatz);
    CHECK(wertAn(leer, 64, Skala::Linear) == 5.0);
    Kurve wild = kurveAusPunkten({{200, 50.0}, {-5, -1.0}, {60, NAN}});
    richteKurve(wild, 0.0, 10.0, ersatz, 21, 108);
    REQUIRE(wild.punkte.size() == 2);
    CHECK(wild.punkte[0].note == 21);
    CHECK(wild.punkte[0].wert == 0.0);
    CHECK(wild.punkte[1].note == 108);
    CHECK(wild.punkte[1].wert == 10.0);
}

TEST_CASE("Eigenzerlegung: A·v = λ·v, Entartung wird gemeldet (L11)")
{
    KleineMatrix a(3);
    const Komplex w[9] = {{1, 2}, {0.5, 0}, {0, 1}, {0.2, 0}, {-1, 0.5}, {0.3, 0.1}, {0, -0.4}, {0.1, 0}, {2, -1}};
    for (int i = 0; i < 9; ++i) a.werte[static_cast<std::size_t>(i)] = w[i];
    const Eigenzerlegung z = zerlegeKlein(a);
    CHECK(z.vollstaendig);
    CHECK_FALSE(z.entartet);
    for (int m = 0; m < 3; ++m)
        for (int i = 0; i < 3; ++i)
        {
            Komplex av(0.0, 0.0);
            for (int j = 0; j < 3; ++j) av += a(i, j) * z.rechts(j, m);
            CHECK(std::abs(av - z.werte[static_cast<std::size_t>(m)] * z.rechts(i, m)) < 1e-12);
        }
    // zwei gleiche Eigenwerte (δ = 0 im Chor): gemeldet
    KleineMatrix b(2);
    b(0, 0) = Komplex(-1.0, 10.0);
    b(1, 1) = Komplex(-1.0, 10.0);
    b(0, 1) = 1e-14;
    const Eigenzerlegung e = zerlegeKlein(b);
    CHECK(e.entartet);
    KleineMatrix d(2);
    d(0, 0) = Komplex(-1.0, 10.0);
    d(1, 1) = Komplex(-1.0, 10.0);
    CHECK(zerlegeKlein(d).entartet);  // diagonal und gleich
    d(1, 1) = Komplex(-1.0, 11.0);
    CHECK_FALSE(zerlegeKlein(d).entartet);
}

TEST_CASE("Fft: pocketfft gleich der DFT, rueckwaerts(vorwaerts) = x, Faltung")
{
    const auto alle = fftUmsetzungen();
    for (std::size_t n : {1u, 2u, 7u, 16u, 45u})
    {
        std::vector<double> x(n);
        for (std::size_t i = 0; i < n; ++i) x[i] = std::sin(0.3 * i) + 0.1 * i;
        std::vector<std::complex<double>> a(n / 2 + 1), b(n / 2 + 1);
        alle[0]->vorwaerts(x, a);
        alle[1]->vorwaerts(x, b);
        for (std::size_t k = 0; k < a.size(); ++k) CHECK(std::abs(a[k] - b[k]) < 1e-9);
        std::vector<double> y(n);
        alle[0]->rueckwaerts(a, y);
        for (std::size_t i = 0; i < n; ++i) CHECK(y[i] == doctest::Approx(x[i]).epsilon(1e-12));
    }
    const std::vector<double> f = falte(fft(), std::vector<double>{1, 2, 3}, std::vector<double>{0, 1, 0.5});
    REQUIRE(f.size() == 5);
    const double soll[5] = {0, 1, 2.5, 4, 1.5};
    for (int i = 0; i < 5; ++i) CHECK(f[static_cast<std::size_t>(i)] == doctest::Approx(soll[i]).epsilon(1e-12));
}

TEST_CASE("SHA-256: Pruefvektoren aus FIPS 180-4 (Anhang, „abc“ und der leere Text)")
{
    const std::string abc = "abc";
    CHECK(sha256({reinterpret_cast<const std::uint8_t*>(abc.data()), abc.size()}) ==
          "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(sha256({}) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    const std::string lang = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    CHECK(sha256({reinterpret_cast<const std::uint8_t*>(lang.data()), lang.size()}) ==
          "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
}

TEST_CASE("Pegel: K-Filter bei 48 kHz wie BS.1770-4 Tab. 1/2, LUFS eines Sinus")
{
    const Biquad k = kFilterKugel(48000);
    CHECK(k.b0 == doctest::Approx(1.53512485958697).epsilon(1e-9));
    CHECK(k.b1 == doctest::Approx(-2.69169618940638).epsilon(1e-9));
    CHECK(k.b2 == doctest::Approx(1.19839281085285).epsilon(1e-9));
    CHECK(k.a1 == doctest::Approx(-1.69065929318241).epsilon(1e-9));
    CHECK(k.a2 == doctest::Approx(0.73248077421585).epsilon(1e-9));
    const Biquad h = kFilterHochpass(48000);
    CHECK(h.a1 == doctest::Approx(-1.99004745483398).epsilon(1e-9));
    CHECK(h.a2 == doctest::Approx(0.99007225036621).epsilon(1e-9));
    // ein Sinus 1 kHz mit Spitze 1 (0 dBFS) hat −3,01 dB mittlere Leistung; die K-Bewertung hebt bei 1 kHz um rund
    // 0,69 dB — zusammen −3,01 dB LUFS (der Versatz −0,691 ist so gewaehlt; BS.1770: „1 kHz sine … −3,01 LKFS“)
    std::vector<float> s(48000 * 4);
    for (std::size_t i = 0; i < s.size(); ++i) s[i] = static_cast<float>(std::sin(2.0 * std::numbers::pi * 1000.0 * i / 48000.0));
    CHECK(lufsNaeherung(s, 48000) == doctest::Approx(-3.01).epsilon(0.005));
}

TEST_CASE("Lautheit nach BS.1770-4 mit Gating (HT14 a) — Faelle nach EBU Tech 3341, auf Mono umgerechnet")
{
    // Tech 3341 legt die Sinusse je Kanal auf −23 dBFS (stereo, beide Kanaele: −23 LUFS). Mono gibt nur einen Kanal —
    // derselbe Pegel waere 3,01 LU leiser; daher hier die Amplitude √2 · 10^(L/20), damit „L dBFS“ L LUFS ergibt.
    const auto sinus = [](double lufs, double sekunden) {
        const double a = std::sqrt(2.0) * std::pow(10.0, lufs / 20.0);
        std::vector<float> s(static_cast<std::size_t>(sekunden * 48000));
        for (std::size_t i = 0; i < s.size(); ++i) s[i] = static_cast<float>(a * std::sin(2.0 * std::numbers::pi * 1000.0 * i / 48000.0));
        return s;
    };
    const auto folge = [](std::initializer_list<std::vector<float>> teile) {
        std::vector<float> s;
        for (const auto& t : teile) s.insert(s.end(), t.begin(), t.end());
        return s;
    };
    // Fall 1 und 2: −23 bzw. −33 LUFS, 20 s
    CHECK(lautheit(sinus(-23.0, 20.0), 48000) == doctest::Approx(-23.0).epsilon(0.1 / 23.0));
    CHECK(lautheit(sinus(-33.0, 20.0), 48000) == doctest::Approx(-33.0).epsilon(0.1 / 33.0));
    // Fall 3: −36 / −23 / −36 dBFS fuer 10 / 60 / 10 s — das relative Gate nimmt die leisen Teile heraus: −23
    CHECK(lautheit(folge({sinus(-36.0, 10.0), sinus(-23.0, 60.0), sinus(-36.0, 10.0)}), 48000) == doctest::Approx(-23.0).epsilon(0.1 / 23.0));
    // Fall 4: −72 / −36 / −23 / −36 / −72 dBFS fuer 10 / 10 / 60 / 10 / 10 s — dazu das absolute Gate: −23
    CHECK(lautheit(folge({sinus(-72.0, 10.0), sinus(-36.0, 10.0), sinus(-23.0, 60.0), sinus(-36.0, 10.0), sinus(-72.0, 10.0)}), 48000) ==
          doctest::Approx(-23.0).epsilon(0.1 / 23.0));
    // ohne Gate (Naeherung) gaebe Fall 3 rund −23,9: der Unterschied, den das Gate macht
    CHECK(lufsNaeherung(folge({sinus(-36.0, 10.0), sinus(-23.0, 60.0), sinus(-36.0, 10.0)}), 48000) < -23.5);
    // Stille und kurze Toene
    CHECK(lautheit(std::vector<float>(48000, 0.0f), 48000) == -300.0);
    CHECK(lautheit(sinus(-23.0, 0.2), 48000) == doctest::Approx(-23.0).epsilon(0.2 / 23.0));
}

TEST_CASE("WAV: hin und zurueck Bit fuer Bit")
{
    const std::string pfad = (std::filesystem::temp_directory_path() / "ic_test.wav").string();
    std::vector<float> w = {0.0f, 0.5f, -0.25f, 1e-7f, -1.0f, 0.125f};
    REQUIRE(schreibeWav(pfad, w, 44100, 2));
    std::vector<float> r;
    int rate = 0, kanaele = 0;
    REQUIRE(leseWav(pfad, r, rate, kanaele));
    CHECK(rate == 44100);
    CHECK(kanaele == 2);
    CHECK(r == w);
    std::filesystem::remove(pfad);
}

TEST_CASE("Modal: Impulsantwort gleich der geschlossenen Loesung, Nachklang aus der schwaechsten Daempfung")
{
    const double fs = 48000.0;
    const Mode m = modeAus(440.0, 2.0, 0.01, 1.0, 1.0, 1.0);
    const std::vector<double> h = impulsantwort({m}, fs, 4800);
    const double sigma = sigmaAusT60(2.0);
    const double omega = 2.0 * std::numbers::pi * 440.0;
    for (int i = 1; i < 4800; i += 97)
    {
        const double t = i / fs;
        CHECK(h[static_cast<std::size_t>(i)] == doctest::Approx(std::exp(-sigma * t) * std::sin(omega * t) / (0.01 * omega)).epsilon(1e-9));
    }
    const Mode kurz = modeAus(880.0, 0.5, 0.01, 1.0, 1.0, 1.0);
    CHECK(std::abs(nachklangFrames({m, kurz}, fs, 60.0, 1 << 30) - static_cast<std::int64_t>(2.0 * fs)) <= 1);
    // Geschwindigkeit als Ausgang: Ableitung der Auslenkung
    const Mode v = modeAus(440.0, 2.0, 0.01, 1.0, 1.0, 1.0, Ausgang::Geschwindigkeit);
    const std::vector<double> hv = impulsantwort({v}, fs, 4800);
    for (int i = 100; i < 4700; i += 211)
        CHECK(hv[static_cast<std::size_t>(i)] == doctest::Approx((h[static_cast<std::size_t>(i + 1)] - h[static_cast<std::size_t>(i - 1)]) * fs / 2.0).epsilon(2e-3));
}

TEST_CASE("Stoss: Kontaktzeit einer Hertz-Feder gegen eine fast starre Wand (geschlossene Loesung)")
{
    // ein Kopf m gegen F = k·δ^(3/2): t_c = 2,9432·(m²/(k²·v))^(1/5) bei starrer Wand (Lehrbuch, aus dem Integral
    // t_c = 2·δ_max/v·∫₀¹ dx/√(1 − x^(5/2)), δ_max = (5mv²/(4k))^(2/5)); Wand: eine Mode mit 10⁶ kg
    Stossanreger a;
    a.masseKg = 0.025;
    a.q0 = 5e7 * std::pow(1e-3, 1.5);  // Rucz & Ulveczki Tab. 1: K_M = 5·10⁷ N/m^1,5 → N/mm^1,5
    a.p = 1.5;
    const double v = 1.0;
    ModalBank bank({modeAus(50.0, 10.0, 1e6, 1.0, 1.0, 1.0)}, 48000.0);
    std::vector<double> summe;
    const StossMessung m = stosse(bank, a, v, summe, 200);
    const double k = 5e7;
    const double tc = 2.9432 * std::pow(a.masseKg * a.masseKg / (k * k * v), 0.2);
    CHECK(m.kontaktS == doctest::Approx(tc).epsilon(0.01));
    // Rucz & Ulveczki 2021, Tab. 1: „Kontakt 0,6 ms bei 1 m/s“ (so im Entwurf, nachgelesen) — dieselbe Groessenordnung
    CHECK(m.kontaktS > 0.4e-3);
    CHECK(m.kontaktS < 0.8e-3);
    CHECK(m.kontakte == 1);
}

TEST_CASE("Balken: frei–frei 1 : 2,757 : 5,404, f1 nach (βL)²·√(EI/ρA)/(2πL²), Kragbalken")
{
    const std::vector<double> v = freiFreiVerhaeltnisse(4);
    CHECK(v[1] == doctest::Approx(2.7565).epsilon(1e-4));
    CHECK(v[2] == doctest::Approx(5.4039).epsilon(1e-4));
    Balken b;
    b.laengeM = 0.5;
    b.eModulPa = 7e10;
    b.dichteKgM3 = 2700.0;
    b.elemente = 80;
    const std::vector<BalkenMode> m = biegeModen(b, 4);
    REQUIRE(m.size() == 4);
    const double ei = 7e10 * 0.05 * std::pow(0.02, 3) / 12.0;
    const double ra = 2700.0 * 0.05 * 0.02;
    const double bl = std::sqrt(v[0]) * 4.730040745;  // βL der ersten Mode
    (void)bl;
    const double f1 = 4.730040745 * 4.730040745 / (2.0 * std::numbers::pi * 0.25) * std::sqrt(ei / ra);
    CHECK(m[0].hz == doctest::Approx(f1).epsilon(1e-4));
    for (int i = 1; i < 4; ++i) CHECK(m[static_cast<std::size_t>(i)].hz / m[0].hz == doctest::Approx(v[static_cast<std::size_t>(i)]).epsilon(1e-3));
    // Knotenlinie der ersten Mode bei 0,2242·L (Lehrbuch)
    double knoten = 0.0;
    for (int i = 1; i <= 40; ++i)
        if (formAn(m[0], b, (i - 1) * 0.25 / 40) * formAn(m[0], b, i * 0.25 / 40) <= 0.0) knoten = (i - 0.5) * 0.25 / 40;
    CHECK(knoten / 0.5 == doctest::Approx(0.2242).epsilon(0.02));
    b.links = Lager::Eingespannt;
    const std::vector<BalkenMode> k = biegeModen(b, 3);
    const std::vector<double> beta = kragBetaL(3);
    CHECK(beta[0] == doctest::Approx(1.8751).epsilon(1e-4));
    for (int i = 0; i < 3; ++i)
        CHECK(k[static_cast<std::size_t>(i)].hz ==
              doctest::Approx(beta[static_cast<std::size_t>(i)] * beta[static_cast<std::size_t>(i)] / (2.0 * std::numbers::pi * 0.25) * std::sqrt(ei / ra)).epsilon(1e-3));
}

TEST_CASE("Luftraum: ein Kolben und eine Oeffnung — gekoppelte Frequenzen wie die geschlossene Loesung")
{
    Luftraum r;
    r.volumenM3 = 14e-3;
    KoerperMode d;
    d.hz = 211.0;
    d.t60S = 1e6;  // fast verlustfrei: die Frequenzen der ungedaempften Loesung
    d.masseKg = 0.05;
    d.flaecheM2 = 0.03;
    r.waende = {d};
    Oeffnung o;
    o.flaecheM2 = 5.7e-3;
    o.hz = 130.0;
    o.q = 1e6;
    r.oeffnungen = {o};
    const LuftraumModen lm = luftraumModen(r);
    REQUIRE(lm.vollstaendig);
    REQUIRE(lm.moden.size() == 2);
    // geschlossen: det(K − ω²M) = 0 mit K = [[mω_d² , αA_dA_h], [αA_dA_h, αA_h²]] (die Wand bei geschlossener
    // Oeffnung schon mit αA_d² gesteift), M = diag(m, m_h)
    const double alpha = r.dichte * r.schall * r.schall / r.volumenM3;
    const double wd = 2.0 * std::numbers::pi * d.hz;
    const double wh = 2.0 * std::numbers::pi * o.hz;
    const double mh = alpha * o.flaecheM2 * o.flaecheM2 / (wh * wh);
    const double k11 = d.masseKg * wd * wd, k12 = alpha * d.flaecheM2 * o.flaecheM2, k22 = alpha * o.flaecheM2 * o.flaecheM2;
    const double a2 = d.masseKg * mh, a1 = -(k11 * mh + k22 * d.masseKg), a0 = k11 * k22 - k12 * k12;
    const double w1 = std::sqrt((-a1 - std::sqrt(a1 * a1 - 4 * a2 * a0)) / (2 * a2));
    const double w2 = std::sqrt((-a1 + std::sqrt(a1 * a1 - 4 * a2 * a0)) / (2 * a2));
    std::vector<double> f;
    for (const Mode& m : lm.moden) f.push_back(m.lambda.imag() / (2.0 * std::numbers::pi));
    std::sort(f.begin(), f.end());
    CHECK(f[0] == doctest::Approx(w1 / (2 * std::numbers::pi)).epsilon(1e-9));
    CHECK(f[1] == doctest::Approx(w2 / (2 * std::numbers::pi)).epsilon(1e-9));
    // die Spitzen liegen unter bzw. ueber den entkoppelten (abstossen)
    CHECK(f[0] < o.hz);
    CHECK(f[1] > d.hz);
    // die Antwort im Zeitbereich trifft den Betrag im Frequenzbereich an der Spitze (Fourier der Impulsantwort)
    r.waende[0].t60S = 1.0;
    r.oeffnungen[0].q = 30.0;
    const LuftraumModen lm2 = luftraumModen(r);
    const std::vector<double> h = impulsantwort(lm2.moden, 48000.0, 96000);
    for (double fp : {100.0, 150.0, 250.0})
    {
        Komplex s(0.0, 0.0);
        for (std::size_t i = 0; i < h.size(); ++i) s += h[i] * std::exp(Komplex(0.0, -2.0 * std::numbers::pi * fp * i / 48000.0));
        s = s / 48000.0 + lm2.durchgriff;
        CHECK(std::abs(s) == doctest::Approx(luftraumBetrag(r, fp)).epsilon(0.02));
    }
}

TEST_CASE("Rohr: verlustfreier Zylinder offen und geschlossen, Kegel als Treppe konvergiert, Kern trifft Z(ω)")
{
    const Luft luft;
    Bohrung b;
    b.abschnitte = {{0.5, 0.01, 0.01}};
    b.verluste = false;
    b.ende = Abschluss::Ideal;
    const std::vector<ImpedanzMode> offen = impedanzModen(b, 1200.0, false, 1.0);
    REQUIRE(offen.size() >= 3);
    for (int n = 1; n <= 3; ++n) CHECK(offen[static_cast<std::size_t>(n - 1)].hz == doctest::Approx(zylinderSpitzeHz(0.5, n, true)).epsilon(1e-5));
    b.ende = Abschluss::Geschlossen;
    const std::vector<ImpedanzMode> zu = impedanzModen(b, 1100.0, false, 1.0);
    REQUIRE(zu.size() >= 3);
    for (int n = 1; n <= 3; ++n) CHECK(zu[static_cast<std::size_t>(n - 1)].hz == doctest::Approx(zylinderSpitzeHz(0.5, n, false)).epsilon(1e-5));
    // mit Verlusten und Abstrahlung: tiefer — Endkorrektur 0,6133·a und die langsamere Welle der Wandverluste
    // (Phasengeschwindigkeit c/(1 + ε), ε wie in `zylinder`)
    b.verluste = true;
    b.ende = Abschluss::Offen;
    const std::vector<ImpedanzMode> echt = impedanzModen(b, 1200.0, false, 1.0);
    const double f0 = luft.schall / (4.0 * (0.5 + 0.6133 * 0.01));
    const double rv = 0.01 * std::sqrt(2.0 * std::numbers::pi * f0 * luft.dichte / luft.zaehigkeit);
    const double eps = (1.0 + (luft.gamma - 1.0) / std::sqrt(luft.prandtl)) / (std::sqrt(2.0) * rv);
    CHECK(echt[0].hz == doctest::Approx(f0 / (1.0 + eps)).epsilon(1e-3));
    CHECK(echt[0].q > 10.0);
    CHECK(echt[0].q < 200.0);
    // ein Kegel: halbe Stufe aendert die Spitzen kaum (die Treppe hat 1 mm)
    Bohrung kegel;
    kegel.abschnitte = {{0.6, 0.005, 0.03}};
    const std::vector<ImpedanzMode> k1 = impedanzModen(kegel, 1500.0, false, 1.0);
    Bohrung kegel2;
    kegel2.abschnitte = {{0.3, 0.005, 0.0175}, {0.3, 0.0175, 0.03}};
    const std::vector<ImpedanzMode> k2 = impedanzModen(kegel2, 1500.0, false, 1.0);
    REQUIRE(k1.size() == k2.size());
    for (std::size_t i = 0; i < k1.size(); ++i) CHECK(k1[i].hz == doctest::Approx(k2[i].hz).epsilon(1e-3));
    // der Kern: Antwort auf einen Flussstoss gleich der Summe der Moden Σ C s/(s² + βs + ω²)
    RohrKern kern(echt, 48000.0);
    std::vector<double> p(48000);
    for (std::size_t i = 0; i < p.size(); ++i)
    {
        const double u = i == 0 ? 48000.0 : 0.0;  // ein Stoss von 1 m³/s·Takt → Fluss·Δt = 1
        p[i] = kern.vorher() + kern.durchgriff() * u;
        kern.nachher(u);
    }
    for (const double f : {echt[0].hz, 0.5 * (echt[0].hz + echt[1].hz)})
    {
        Komplex s(0.0, 0.0);
        for (std::size_t i = 0; i < p.size(); ++i) s += p[i] * std::exp(Komplex(0.0, -2.0 * std::numbers::pi * f * i / 48000.0));
        s /= 48000.0;
        Komplex z(0.0, 0.0);
        const Komplex sw(0.0, 2.0 * std::numbers::pi * f);
        for (const ImpedanzMode& m : echt)
        {
            const double wn = 2.0 * std::numbers::pi * m.hz;
            z += m.spitze * wn / m.q * sw / (sw * sw + wn / m.q * sw + wn * wn);
        }
        CHECK(std::abs(s) == doctest::Approx(std::abs(z)).epsilon(0.02));
    }
}

TEST_CASE("Pool: parallel gerechnet gleich wie der Reihe nach (Determinismus)")
{
    std::vector<double> a(64), b(64);
    const auto rechne = [](std::size_t i) {
        double s = 0.0;
        for (int k = 0; k < 10000; ++k) s += std::sin(0.001 * k * (i + 1));
        return s;
    };
    parallelJe(64, [&](std::size_t i) { a[i] = rechne(i); }, 1);
    parallelJe(64, [&](std::size_t i) { b[i] = rechne(i); }, 8);
    CHECK(a == b);
}
