/**
 * @file test_hammer_rest.cpp
 * @brief Klasse 1 (Rest): Upright, Honky-tonk, CP-70 auf dem Fluegel-Kern; Clavinet, Hackbrett, Cimbalom auf dem
 *        Saitenkern — analytische Faelle, Literaturwerte (Gabrielli et al. 2013, Peterson 2005), Pedale, Grenzen,
 *        Determinismus, Rechenzeit, Abdruck
 */
#include "doctest.h"

#include "gemeinsam/Fft.hpp"
#include "klassen/01_hammer_saite/HammerTon.hpp"
#include "klassen/01_hammer_saite/Saite.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <numbers>

using namespace ic;

namespace
{

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

hammer::HammerAnschlag hammerAnschlag(int taste, int stufe, double haltenS = 0.5)
{
    hammer::HammerAnschlag a;
    a.taste = taste;
    a.hz = hzVon(taste);
    a.stufe = stufe;
    a.halten = std::llround(haltenS * a.rate);
    a.gedaempft = taste < hammer::kUngedaempftAbVorgabe;
    return a;
}

saite::SaitenAnschlag saitenAnschlag(int note, int stufe, double haltenS = 0.5)
{
    saite::SaitenAnschlag a;
    a.note = note;
    a.hz = hzVon(note);
    a.stufe = stufe;
    a.halten = std::llround(haltenS * a.rate);
    return a;
}

/// Die staerkste Frequenz zwischen `von` und `bis` in einem Ausschnitt (Hann, Nullen auf 2^18).
double staerksteHz(const std::vector<float>& x, std::size_t ab, std::size_t n, double von, double bis)
{
    const std::size_t lang = 1u << 18;
    std::vector<double> z(lang, 0.0);
    for (std::size_t i = 0; i < n && ab + i < x.size(); ++i) z[i] = x[ab + i] * (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * i / n));
    std::vector<std::complex<double>> s(lang / 2 + 1);
    fft().vorwaerts(z, s);
    std::size_t beste = 0;
    for (std::size_t k = 1; k < s.size(); ++k)
    {
        const double f = k * 48000.0 / lang;
        if (f < von || f > bis) continue;
        if (beste == 0 || std::abs(s[k]) > std::abs(s[beste])) beste = k;
    }
    return beste * 48000.0 / lang;
}

double schwerpunkt(const std::vector<float>& t, std::size_t n)
{
    std::vector<double> x(n, 0.0);
    for (std::size_t i = 0; i < n && i < t.size(); ++i) x[i] = t[i];
    std::vector<std::complex<double>> sp(n / 2 + 1);
    fft().vorwaerts(x, sp);
    double a = 0.0, b = 0.0;
    for (std::size_t k = 1; k < sp.size(); ++k)
    {
        a += k * std::norm(sp[k]);
        b += std::norm(sp[k]);
    }
    return a / b * 48000.0 / n;
}

}  // namespace

TEST_CASE("Upright: Bass gekappt auf 1,189 m, gleiche Spannung → µ und B mal (L_F/L_U)²; Tastatur Soft/Practice/Sustain")
{
    const hammer::HammerSaite& f = hammer::hammerSaiteVorgabe();
    const hammer::HammerSaite u = hammer::baueVorgabeUpright();
    for (const int t : {24, 30, 36})
    {
        const double lf = wertAn(f.saitenLaengeM, t, Skala::Logarithmisch);
        const double lu = wertAn(u.saitenLaengeM, t, Skala::Logarithmisch);
        CHECK(lu == doctest::Approx(std::min(lf, 46.8 * 0.0254)).epsilon(1e-9));
        const double faktor = (lf / lu) * (lf / lu);
        CHECK(wertAn(u.inharmonizitaet, t, Skala::Logarithmisch) == doctest::Approx(wertAn(f.inharmonizitaet, t, Skala::Logarithmisch) * faktor).epsilon(1e-9));
        // T0 = 4L²f0²µ bleibt gleich
        const double tf = lf * lf * wertAn(f.saitenMasseGm, t, Skala::Logarithmisch);
        const double tu = lu * lu * wertAn(u.saitenMasseGm, t, Skala::Logarithmisch);
        CHECK(tu == doctest::Approx(tf).epsilon(1e-9));
    }
    const hammer::Tastatur k = hammer::tastaturUpright();
    CHECK(k.links == hammer::PedalArt::Leise);
    CHECK(k.mitte == hammer::PedalArt::Moderator);
    hammer::Tastatur b = k;
    b.mitte = hammer::PedalArt::BassSustain;
    b.bassSustainBis = 48;
    hammer::richteTastatur(b);
    CHECK(b.mitte == hammer::PedalArt::BassSustain);
    CHECK_FALSE(hammer::daempferWirkt(b, 40, false, true));
    CHECK(hammer::daempferWirkt(b, 50, false, true));
    CHECK(hammer::daempferWirkt(b, 40, false, false));
    CHECK_FALSE(hammer::daempferWirkt(b, 95, false, false));  // ohne Daempfer ab 89
}

TEST_CASE("Upright: Moderator macht leiser und dunkler; Soft mit Faktor 1 ist Bit fuer Bit der Ton ohne (neutral)")
{
    const hammer::HammerSaite u = hammer::baueVorgabeUpright();
    const hammer::Tastatur k = hammer::tastaturUpright();
    hammer::HammerAnschlag a = hammerAnschlag(60, -6);
    std::vector<float> ohne, mit, soft;
    const auto frames = a.halten + hammer::hammerNachklang(u, a, 96000);
    hammer::rechneHammerTon(u, a, ohne, frames);
    hammer::HammerAnschlag m = a;
    m.moderatorSteife = k.moderatorSteife;
    m.moderatorExponent = k.moderatorExponent;
    hammer::rechneHammerTon(u, m, mit, frames);
    CHECK(rms(mit) < rms(ohne));
    CHECK(schwerpunkt(mit, 16384) < schwerpunkt(ohne, 16384));
    hammer::HammerAnschlag s = a;
    s.tempoFaktor = k.softFaktor;
    hammer::rechneHammerTon(u, s, soft, frames);
    CHECK(soft == ohne);
}

TEST_CASE("Honky-tonk: ±1 Hz bei A4 — die aeusseren Saiten 2 Hz auseinander; CP-70: eine Saite bis MIDI 42")
{
    const hammer::HammerSaite h = hammer::baueVorgabeHonkytonk();
    const double cent = wertAn(h.verstimmungCent, 69, Skala::Linear);
    CHECK(440.0 * (std::exp2(cent / 2400.0) - std::exp2(-cent / 2400.0)) == doctest::Approx(2.0).epsilon(1e-3));
    // die Moden des Chors an A4: zwei Frequenzen 2 Hz auseinander (die Eigenwerte des Chors, Bank 2010)
    const auto t = hammer::hammerTeiltonModen(h, hammerAnschlag(69, -12));
    double lo = 1e9, hi = 0.0;
    for (const hammer::EbenenMode& m : t[0].moden)
        if (std::abs(m.anteil) > 0.05)
        {
            lo = std::min(lo, m.lambda.imag());
            hi = std::max(hi, m.lambda.imag());
        }
    CHECK((hi - lo) / (2.0 * std::numbers::pi) == doctest::Approx(2.0).epsilon(0.1));
    const hammer::HammerSaite c = hammer::baueVorgabeCp70();
    CHECK(hammer::chorAn(c, 42) == 1);
    CHECK(hammer::chorAn(c, 43) == 2);
}

TEST_CASE("Clavinet: nach dem Loslassen drei Halbtoene tiefer (Gabrielli et al. 2013 S. 6); Tastendruck mit Grenze 0")
{
    const saite::SaitenSpiel c = saite::baueVorgabeClavinet();
    saite::SaitenAnschlag a = saitenAnschlag(57, -6, 0.3);
    std::vector<float> ton;
    saite::SaitenMessung m;
    saite::rechneSaitenTon(c, a, ton, a.rate, &m);
    const double f = a.hz;
    CHECK(m.hzNachLoslassen == doctest::Approx(f * std::exp2(-3.0 / 12.0)).epsilon(2e-3));
    // im Ton: vor dem Loslassen die Note, danach drei Halbtoene tiefer
    CHECK(staerksteHz(ton, 2400, 9600, 0.8 * f, 1.2 * f) == doctest::Approx(f).epsilon(0.005));
    CHECK(staerksteHz(ton, a.halten + 480, 4800, 0.7 * f, 0.95 * f) == doctest::Approx(f * std::exp2(-0.25)).epsilon(0.01));
    // der Tastendruck: Ort und Format da, Grenze 0 → derselbe Ton
    saite::SaitenAnschlag d = a;
    d.tastendruckCent = 30.0;
    std::vector<float> gedrueckt;
    saite::rechneSaitenTon(c, d, gedrueckt, a.rate);
    CHECK(gedrueckt == ton);
    // der Daempferschieber kuerzt den Ton
    saite::SaitenAnschlag s = a;
    s.schieber = 1.0;
    s.halten = a.rate;
    a.halten = a.rate;
    std::vector<float> mit, ohne;
    saite::rechneSaitenTon(c, s, mit, a.rate);
    saite::rechneSaitenTon(c, a, ohne, a.rate);
    CHECK(rms(std::span<const float>(mit).subspan(24000, 12000)) < 0.1 * rms(std::span<const float>(ohne).subspan(24000, 12000)));
    // heller mit der Staerke (kuerzerer Stoss der Tangente)
    std::vector<float> leise, laut;
    saite::rechneSaitenTon(c, saitenAnschlag(45, -20), leise, 16384);
    saite::rechneSaitenTon(c, saitenAnschlag(45, 0), laut, 16384);
    CHECK(schwerpunkt(laut, 16384) > schwerpunkt(leise, 16384));
}

TEST_CASE("Hackbrett: Peterson 2005 — Spannung, Laenge, Radius ergeben die Tonhoehe auf 5 %; Steg 3 : 2 koppelt jeden dritten Teilton")
{
    // f = (1/2L)·√(T/µ) aus der Tabelle (nur Bericht) gegen die Note — prueft die Lesart „rad.“ = Radius (die letzte
    // Zeile „G#3 196“: 196 Hz ist G3, MIDI 55)
    const struct
    {
        int note;
        double r, l, t;
    } z[] = {{55, 0.012, 21, 21}, {62, 0.011, 18, 29}, {67, 0.010, 15, 30}, {74, 0.010, 12, 43}, {79, 0.009, 9, 35}};
    for (const auto& e : z)
    {
        const double mu = 7850.0 * std::numbers::pi * std::pow(e.r * 0.0254, 2);
        const double f = std::sqrt(e.t * 4.44822 / mu) / (2.0 * e.l * 0.0254);
        CAPTURE(e.note);
        CHECK(f == doctest::Approx(hzVon(e.note)).epsilon(0.05));
    }
    const saite::SaitenSpiel h = saite::baueVorgabeHackbrett();
    std::vector<float> mit, ohne;
    saite::SaitenSpiel ohneSteg = h;
    ohneSteg.stegTeilung = 0.0;
    const saite::SaitenAnschlag a = saitenAnschlag(62, -6, 2.0);
    saite::rechneSaitenTon(h, a, mit, 96000);
    saite::rechneSaitenTon(ohneSteg, a, ohne, 96000);
    CHECK(mit != ohne);
    // am dritten Teilton (= zweiter des kurzen Teils) fliesst Energie in den anderen Abschnitt: dort sinkt der Pegel
    // des angeschlagenen schneller — der dritte Teilton faellt mit Steg staerker als ohne
    const double f3 = 3.0 * a.hz;
    const auto band = [&](const std::vector<float>& t, std::size_t ab) {
        Komplex s(0.0, 0.0);
        for (std::size_t i = 0; i < 9600; ++i) s += static_cast<double>(t[ab + i]) * std::exp(Komplex(0.0, -2.0 * std::numbers::pi * f3 * (ab + i) / 48000.0));
        return std::abs(s);
    };
    const double abfallMit = dB(band(mit, 4800) / band(mit, 81600));
    const double abfallOhne = dB(band(ohne, 4800) / band(ohne, 81600));
    CHECK(abfallMit != doctest::Approx(abfallOhne).epsilon(1e-3));
}

TEST_CASE("Clavinet: Wahl der Tonabnehmer (Gabrielli 2011/2013) und Verstaerker-Shelves (−3 dB an 130 Hz)")
{
    const saite::SaitenSpiel c = saite::baueVorgabeClavinet();
    REQUIRE(c.verstaerker);
    saite::SaitenAnschlag a = saitenAnschlag(29, -6, 1.0);  // F1, 43,7 Hz
    std::array<std::vector<float>, 4> t;
    const saite::AbnehmerWahl wahl[4] = {saite::AbnehmerWahl::Beide, saite::AbnehmerWahl::Steg, saite::AbnehmerWahl::Mitte,
                                         saite::AbnehmerWahl::Gegenphase};
    for (int i = 0; i < 4; ++i)
    {
        a.abnehmerWahl = wahl[i];
        saite::rechneSaitenTon(c, a, t[static_cast<std::size_t>(i)], 48000);
    }
    double abwSumme = 0.0, abwDiff = 0.0, spitze = 0.0;
    for (std::size_t i = 0; i < t[0].size(); ++i)
    {
        abwSumme = std::max(abwSumme, std::abs(double(t[0][i]) - (double(t[1][i]) + t[2][i])));
        abwDiff = std::max(abwDiff, std::abs(double(t[3][i]) - (double(t[1][i]) - t[2][i])));
        spitze = std::max(spitze, std::abs(double(t[0][i])));
    }
    CHECK(abwSumme < 1e-5 * spitze);
    CHECK(abwDiff < 1e-5 * spitze);
    CHECK(t[1] != t[2]);
    // der Verstaerker: am Grundton von F1 (weit unter 130 Hz) rund −3 dB
    saite::SaitenSpiel ohne = c;
    ohne.verstaerker = false;
    a.abnehmerWahl = saite::AbnehmerWahl::Beide;
    std::vector<float> o;
    saite::rechneSaitenTon(ohne, a, o, 48000);
    const auto band = [](const std::vector<float>& x, double f) {
        std::complex<double> s(0.0, 0.0);
        for (std::size_t i = 4800; i < 4800 + 38400; ++i)
            s += (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * double(i - 4800) / 38400.0)) * double(x[i]) *
                 std::exp(std::complex<double>(0.0, -2.0 * std::numbers::pi * f * double(i) / 48000.0));
        return std::abs(s);
    };
    const double db = 20.0 * std::log10(band(t[0], a.hz) / band(o, a.hz));
    std::printf("Clavinet F1 am Grundton mit Verstaerker: %+.2f dB\n", db);
    CHECK(db == doctest::Approx(-3.0).epsilon(0.1));
}

TEST_CASE("Hackbrett je Clip: die anderen Choere schwingen mit (T2 a, K2 F-3 a); Cimbalom ohne Pedal nicht")
{
    const saite::SaitenSpiel h = saite::baueVorgabeHackbrett();
    REQUIRE(h.mitschwingen);
    const saite::SaitenAnschlag a = saitenAnschlag(60, -6, 4.0);
    std::vector<float> je, clip;
    saite::rechneSaitenTon(h, a, je, 4 * 48000);
    saite::rechneSaitenClip(h, {{0.0, a}}, clip, 4 * 48000);
    CHECK(je != clip);
    // nach 2,5 s ist der Grundton mit Mitschwingen staerker: die Saite C3 (2. Teilton auf C4) gibt Energie zurueck
    const auto band = [](const std::vector<float>& t, std::size_t ab, double f) {
        std::complex<double> s(0.0, 0.0);
        for (std::size_t i = 0; i < 48000; ++i)
            s += (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * i / 48000.0)) * double(t[ab + i]) *
                 std::exp(std::complex<double>(0.0, -2.0 * std::numbers::pi * f * double(ab + i) / 48000.0));
        return std::abs(s);
    };
    const double mitG = band(clip, 120000, a.hz), ohneG = band(je, 120000, a.hz);
    std::printf("Hackbrett C4 nach 2,5 s am Grundton: mit Mitschwingen %+.1f dB\n", 20.0 * std::log10(mitG / ohneG));
    CHECK(mitG > 1.05 * ohneG);
    // zwei Noten: der Clip ist die Summe (linear)
    std::vector<float> zwei, c1, c2;
    const saite::SaitenAnschlag b = saitenAnschlag(67, -6, 4.0);
    saite::rechneSaitenClip(h, {{0.0, a}, {0.5, b}}, zwei, 2 * 48000);
    saite::rechneSaitenClip(h, {{0.0, a}}, c1, 2 * 48000);
    saite::rechneSaitenClip(h, {{0.5, b}}, c2, 2 * 48000);
    double abw = 0.0;
    for (std::size_t i = 0; i < zwei.size(); ++i) abw = std::max(abw, std::abs(double(zwei[i]) - (double(c1[i]) + c2[i])));
    CHECK(abw < 1e-6);
    const std::string ist = abdruck(zwei);
    CHECK(test::erwarteterAbdruck("saite_Hackbrett_clip_C4_G4", ist) == ist);
    // Cimbalom ohne Pedal: die Daempferleiste liegt auf allen Choeren — der Clip ist der Ton
    const saite::SaitenSpiel c = saite::baueVorgabeCimbalom();
    std::vector<float> cj, cc;
    saite::rechneSaitenTon(c, a, cj, 48000);
    saite::rechneSaitenClip(c, {{0.0, a}}, cc, 48000);
    CHECK(cj == cc);
}

TEST_CASE("Klasse 1 Rest: Determinismus, Abdruck, Rechenzeit")
{
    struct Ton
    {
        std::string name;
        std::function<void(std::vector<float>&, int note, int stufe, std::int64_t frames)> rechne;
        int tief;
        bool deluxe = false;
    };
    const hammer::HammerSaite up = hammer::baueVorgabeUpright(), ht = hammer::baueVorgabeHonkytonk(), cp = hammer::baueVorgabeCp70();
    const saite::SaitenSpiel cl = saite::baueVorgabeClavinet(), hb = saite::baueVorgabeHackbrett(), cb = saite::baueVorgabeCimbalom();
    const auto ham = [](const hammer::HammerSaite& s) {
        return [&s](std::vector<float>& t, int note, int stufe, std::int64_t frames) {
            hammer::rechneHammerTon(s, hammerAnschlag(note, stufe), t, frames);
        };
    };
    const auto sai = [](const saite::SaitenSpiel& s) {
        return [&s](std::vector<float>& t, int note, int stufe, std::int64_t frames) {
            saite::rechneSaitenTon(s, saitenAnschlag(note, stufe), t, frames);
        };
    };
    const std::vector<Ton> alle = {{"Upright", ham(up), 21}, {"Honkytonk", ham(ht), 21}, {"CP70", ham(cp), 28},
                                   {"Clavinet", sai(cl), cl.noteMin}, {"Hackbrett", sai(hb), hb.noteMin, true}, {"Cimbalom", sai(cb), cb.noteMin, true}};
    std::vector<std::vector<float>> a(alle.size()), b(alle.size());
    for (std::size_t i = 0; i < alle.size(); ++i) alle[i].rechne(a[i], 60, -12, 48000);
    parallelJe(alle.size(), [&](std::size_t i) { alle[i].rechne(b[i], 60, -12, 48000); }, 3);
    for (std::size_t i = 0; i < alle.size(); ++i)
    {
        CAPTURE(alle[i].name);
        CHECK(a[i] == b[i]);
        CHECK(spitze(a[i]) > 0.0);
        const std::string ist = abdruck(a[i]);
        CHECK(test::erwarteterAbdruck("klasse1_" + alle[i].name + "_m60_a-12", ist) == ist);
        std::vector<float> t;
        const double anteil = echtzeitAnteil([&] { alle[i].rechne(t, alle[i].tief, 0, 96000); }, 2.0);
        std::printf("Rechenzeit %-10s tiefster Ton %3d: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", alle[i].name.c_str(), alle[i].tief,
                    100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
        // Deluxe (F-1 a) darf darueber liegen; berichtet wird die Zahl
        if (!alle[i].deluxe) CHECK(anteil < test::echtzeitGrenzeHier());
    }
}
