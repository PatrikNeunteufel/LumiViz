/**
 * @file test_stab.cpp
 * @brief Klasse 4 Platte und Stab: analytische Faelle, Literaturwerte (Rucz & Ulveczki Tab. 1, Falaize & Hélie,
 *        Chapman), Grenzen, Determinismus, Rechenzeit, Abdruck (AUFTRAG_BAU §4 Punkt 3 a–f)
 */
#include "doctest.h"

#include "gemeinsam/Balken.hpp"
#include "gemeinsam/Fft.hpp"
#include "klassen/04_platte_stab/Stab.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <cstdio>
#include <numbers>

using namespace ic;
using namespace ic::stab;

namespace
{

StabAnschlag anschlag(int note, int stufe)
{
    StabAnschlag a;
    a.note = note;
    a.hz = 440.0 * std::exp2((note - 69) / 12.0);
    a.stufe = stufe;
    return a;
}

/// Der Betrag der DFT bei f ueber einen Ausschnitt (Hann-Fenster).
double bandBetrag(const std::vector<float>& x, std::size_t ab, std::size_t n, double f, double fs)
{
    Komplex s(0.0, 0.0);
    for (std::size_t i = 0; i < n && ab + i < x.size(); ++i)
    {
        const double w = 0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * i / n);
        s += w * x[ab + i] * std::exp(Komplex(0.0, -2.0 * std::numbers::pi * f * (ab + i) / fs));
    }
    return std::abs(s);
}

const std::vector<StabSpiel>& alle()
{
    static const std::vector<StabSpiel> a = alleStabSpiele();
    return a;
}

const StabSpiel& instrument(const char* name)
{
    for (const StabSpiel& s : alle())
        if (s.name == name) return s;
    return alle().front();
}

}  // namespace

TEST_CASE("Stab: Moden aus den Kurven — 1 : 4 : 10 (Marimba), 1 : 3 : 9 (Xylophon), darueber der Balken")
{
    const StabModen m = stabModen(instrument("Marimba"), anschlag(60, 0));
    REQUIRE(m.stab.size() >= 4);
    CHECK(m.stab[1].hz / m.stab[0].hz == doctest::Approx(4.0));
    CHECK(m.stab[2].hz / m.stab[0].hz == doctest::Approx(10.0));
    const std::vector<double> v = freiFreiVerhaeltnisse(4);
    CHECK(m.stab[3].hz / m.stab[2].hz == doctest::Approx(v[3] / v[2]).epsilon(1e-3));
    const StabModen x = stabModen(instrument("Xylophon"), anschlag(72, 0));
    CHECK(x.stab[1].hz / x.stab[0].hz == doctest::Approx(3.0));
    CHECK(x.stab[2].hz / x.stab[0].hz == doctest::Approx(9.0));
    // Mitte angeschlagen: die antisymmetrische zweite Mode bekommt nichts (La Favre, nur Bericht; hier Rechnung)
    CHECK(std::abs(m.stab[1].form) < 1e-6 * std::abs(m.stab[0].form));
    // das geschlossene Rohr: Partnermoden bei 1, 3, 5 … — beim Xylophon trifft 3 die zweite Stabmode (1 : 3 : 9)
    REQUIRE(x.partner.size() >= 2);
    CHECK(x.stab[0].partner == 0);
    CHECK(x.stab[1].partner == 1);
    CHECK(x.stab[2].partner == 4);  // 9·f_r
    // bei der Marimba haben nur Mode 1 (1) und keine weitere einen Partner (4 und 10 sind gerade)
    CHECK(m.stab[0].partner == 0);
    CHECK(m.stab[1].partner == -1);
}

TEST_CASE("Stab gegen Rucz & Ulveczki 2021: T60 ohne Rohr „close to 10 s“, mit Rohr 0,7 s, +15 dB, Kontakt 0,6 ms")
{
    const StabSpiel& s = instrument("Xylophon");
    StabAnschlag a = anschlag(65, -6);
    a.hz = 348.8;  // der Stab der Arbeit
    const StabModen m = stabModen(s, a);
    // ohne Rohr: ξ = 2,57e-4 → 12,3 s; die Arbeit sagt „close to 10 s“ (gerechnet mit ihrem FE-Modell)
    CHECK(m.stab[0].t60S == doctest::Approx(kLn1000 / (2.57e-4 * 2 * std::numbers::pi * 348.8)).epsilon(2e-3));  // die Kurve haengt an F4 = 349,2 Hz
    CHECK(m.stab[0].t60S > 9.0);
    CHECK(m.stab[0].t60S < 13.0);
    // mit Rohr: das Paar ueber der kritischen Kopplung traegt das Mittel der Daempfungen → T60 = 0,71 s
    const double sigmaPaar = 0.5 * (sigmaAusT60(m.stab[0].t60S) + sigmaAusT60(m.partner[0].t60S));
    CHECK(kLn1000 / sigmaPaar == doctest::Approx(0.7).epsilon(0.03));
    // am Ton: der Grundton faellt zwischen 0,2 und 0,6 s um 0,4 s · 60/0,71 dB
    std::vector<float> ton;
    rechneStabTon(s, a, ton, 48000);
    const double f0 = m.stab[0].hz;
    const double frueh = bandBetrag(ton, 9600, 4800, f0, 48000.0);
    const double spaet = bandBetrag(ton, 28800, 4800, f0, 48000.0);
    const double abfall = dB(frueh / spaet);
    CHECK(abfall == doctest::Approx(0.4 * 60.0 / (kLn1000 / sigmaPaar)).epsilon(0.15));
    // +15 dB (gesetzt, Probe): mit Rohr gegen ohne, Effektivwert der ersten 0,5 s
    StabSpiel ohne = s;
    ohne.partner = PartnerArt::Keiner;
    std::vector<float> t1, t2;
    rechneStabTon(s, a, t1, 24000);
    rechneStabTon(ohne, a, t2, 24000);
    CHECK(dB(rms(t1) / rms(t2)) == doctest::Approx(15.0).epsilon(0.01));
    // der Schlaegel bei 1 m/s: Kontakt 0,6 ms (Tab. 1)
    StabSpiel eins = s;
    eins.tempoMax = 1.0;
    StabMessung messung;
    StabAnschlag b = a;
    b.stufe = 0;
    rechneStabTon(eins, b, ton, 4800, &messung);
    CHECK(messung.kontaktS == doctest::Approx(0.6e-3).epsilon(0.15));
    CHECK(messung.kontakte == 1);
}

TEST_CASE("Stab: Konvergenz des Kontakts in der Ueberabtastung (64 gegen 256)")
{
    StabSpiel s = instrument("Marimba");
    std::vector<float> a, b;
    StabMessung ma, mb;
    rechneStabTon(s, anschlag(60, 0), a, 4800, &ma);
    s.ueberabtastung = 256;
    rechneStabTon(s, anschlag(60, 0), b, 4800, &mb);
    CHECK(ma.kraftSpitzeN == doctest::Approx(mb.kraftSpitzeN).epsilon(0.01));
    CHECK(rms(a) == doctest::Approx(rms(b)).epsilon(0.01));
}

TEST_CASE("Rhodes-Tine gegen Falaize & Hélie 2015: Stahl r = 2 mm, 7,83 cm → 440 Hz (Kragbalken, FE)")
{
    // Kreis r: I/A = r²/4; ein Quadrat mit h = r·√3 hat dasselbe I/A (h²/12)
    Balken b;
    b.laengeM = 0.0783;
    b.eModulPa = 180e9;
    b.dichteKgM3 = 7750.0;
    b.hoeheM = [](double) { return 0.002 * std::sqrt(3.0); };
    b.links = Lager::Eingespannt;
    b.elemente = 60;
    const std::vector<BalkenMode> m = biegeModen(b, 2);
    CHECK(m[0].hz == doctest::Approx(440.0).epsilon(0.005));
}

TEST_CASE("Stab: Motor moduliert den Grundton mit der doppelten Drehzahl; die Phase laeuft durch")
{
    const StabSpiel& v = instrument("Vibraphon");
    StabAnschlag a = anschlag(65, -6);
    a.motor = {{-1.0, 4.0}};  // laeuft schon vor dem Anschlag mit 4 U/s
    std::vector<float> ton;
    rechneStabTon(v, a, ton, 96000);
    // Huellkurve |x| geglaettet ueber 5 ms, Spektrum der Huelle: Spitze bei 8 Hz
    std::vector<double> huelle;
    for (std::size_t i = 4800; i + 240 < ton.size(); i += 240)
    {
        double s = 0.0;
        for (std::size_t k = 0; k < 240; ++k) s += std::abs(ton[i + k]);
        huelle.push_back(s);
    }
    const auto amp = [&](double f) {
        Komplex s(0.0, 0.0);
        double mittel = 0.0;
        for (const double h : huelle) mittel += h;
        mittel /= static_cast<double>(huelle.size());
        for (std::size_t i = 0; i < huelle.size(); ++i) s += (huelle[i] - mittel) * std::exp(Komplex(0.0, -2.0 * std::numbers::pi * f * i * 240.0 / 48000.0));
        return std::abs(s);
    };
    CHECK(amp(8.0) > 3.0 * amp(5.0));
    CHECK(amp(8.0) > 3.0 * amp(12.0));
    // ohne Motor keine Modulation
    StabAnschlag ohne = anschlag(65, -6);
    std::vector<float> ruhig;
    rechneStabTon(v, ohne, ruhig, 96000);
    CHECK(ruhig != ton);
}

TEST_CASE("Stab: die Daempferleiste dampft ab der Stelle, das halbe Pedal halb so stark")
{
    const StabSpiel& v = instrument("Vibraphon");
    StabAnschlag a = anschlag(69, -6);
    a.daempferAb = 24000;
    std::vector<float> voll, halb, frei;
    rechneStabTon(v, a, voll, 72000);
    a.andruck = 0.5;
    rechneStabTon(v, a, halb, 72000);
    a.daempferAb = -1;
    rechneStabTon(v, a, frei, 72000);
    const auto spaet = [](const std::vector<float>& t) { return rms(std::span<const float>(t).subspan(36000, 12000)); };
    CHECK(spaet(voll) < spaet(halb));
    CHECK(spaet(halb) < spaet(frei));
    CHECK(dB(spaet(frei) / spaet(voll)) > 20.0);
}

TEST_CASE("Rhodes: der Tonabnehmer klirrt mit der Staerke (Harmonische 2 gegen 1 steigt von pp nach ff)")
{
    const StabSpiel& r = instrument("Rhodes");
    const auto klirr = [&](int stufe) {
        std::vector<float> ton;
        StabAnschlag a = anschlag(57, stufe);
        rechneStabTon(r, a, ton, 24000);
        return bandBetrag(ton, 4800, 9600, 2 * a.hz, 48000.0) / bandBetrag(ton, 4800, 9600, a.hz, 48000.0);
    };
    CHECK(klirr(0) > 1.5 * klirr(-30));
}

TEST_CASE("Stab: die Helligkeit steigt mit der Staerke (kuerzerer Kontakt, M4 qualitativ)")
{
    const auto schwerpunkt = [](const std::vector<float>& t) {
        std::vector<double> x(8192, 0.0);
        for (std::size_t i = 0; i < x.size() && i < t.size(); ++i) x[i] = t[i];
        std::vector<std::complex<double>> sp(x.size() / 2 + 1);
        fft().vorwaerts(x, sp);
        double a = 0.0, b = 0.0;
        for (std::size_t k = 1; k < sp.size(); ++k)
        {
            a += k * std::norm(sp[k]);
            b += std::norm(sp[k]);
        }
        return a / b * 48000.0 / 8192.0;
    };
    for (const char* name : {"Marimba", "Xylophon", "Glockenspiel"})
    {
        CAPTURE(name);
        std::vector<float> leise, laut;
        rechneStabTon(instrument(name), anschlag(instrument(name).noteMin + 12, -30), leise, 8192);
        rechneStabTon(instrument(name), anschlag(instrument(name).noteMin + 12, 0), laut, 8192);
        CHECK(schwerpunkt(laut) > schwerpunkt(leise));
    }
}

TEST_CASE("Stab: Grenzen klemmen, auch die Entartung der Kopplung (L11)")
{
    StabSpiel s = instrument("Marimba");
    const StabSpiel v = s;
    s.verhaeltnis2 = kurveAusPunkten({{60, 100.0}});
    s.kopplungJeKritisch = kurveAusPunkten({{60, 1.0}});
    s.t60S = Kurve{};
    s.ueberabtastung = 0;
    richteStabSpiel(s, v);
    CHECK(wertAn(s.verhaeltnis2, 60, Skala::Linear) == 20.0);
    CHECK(wertAn(s.kopplungJeKritisch, 60, Skala::Linear) == 1.05);
    CHECK(s.t60S == v.t60S);
    CHECK(s.ueberabtastung == 1);
}

TEST_CASE("Vibraphon gestrichen (Bogen × Stab, T1 a): obere Lage bei pp und mf auf ±10 Cent; C4 ohne Resonator auch")
{
    const StabSpiel v = baueVorgabeVibraphon();
    REQUIRE(v.bogen);
    const auto gestrichen = [](int note, double e) {
        StabAnschlag a;
        a.note = note;
        a.hz = 440.0 * std::exp2((note - 69) / 12.0);
        a.gestrichen = true;
        a.expression = e;
        a.haltenS = 1.0;
        return a;
    };
    int getroffen = 0, gesamt = 0;
    for (int note = 63; note <= v.noteMax; note += 3)
        for (const double e : {0.0, 0.5})
        {
            const StabAnschlag a = gestrichen(note, e);
            std::vector<float> t;
            StabMessung m;
            rechneStabTon(v, a, t, 48000, &m);
            ++gesamt;
            const double c = m.schwingt ? 1200.0 * std::log2(m.spielHz / a.hz) : 1e9;
            if (std::abs(c) < 10.0 && m.haftAnteil > 0.05) ++getroffen;
        }
    std::printf("Vibraphon gestrichen E♭4 … F6, pp und mf: %d von %d auf ±10 Cent\n", getroffen, gesamt);
    CHECK(getroffen * 10 >= gesamt * 9);
    // ohne Resonator spielt auch die tiefe Lage (mit ihm rastet sie auf die obere Mode der Kopplung ein, offen.md)
    StabSpiel ohne = baueVorgabeVibraphon(false);
    ohne.partner = PartnerArt::Keiner;
    StabAnschlag a = gestrichen(60, 0.5);
    a.bogenKraftN = 1.6;
    a.bogenVbMS = 0.18;
    std::vector<float> t;
    StabMessung m;
    rechneStabTon(ohne, a, t, 48000, &m);
    REQUIRE(m.schwingt);
    CHECK(std::abs(1200.0 * std::log2(m.spielHz / a.hz)) < 10.0);
    CHECK(m.haftAnteil > 0.05);
    // Abdruck
    std::vector<float> g;
    rechneStabTon(v, gestrichen(72, 0.5), g, 48000);
    const std::string ist = abdruck(g);
    CHECK(test::erwarteterAbdruck("stab_Vibraphon_gestrichen_m72_mf", ist) == ist);
}

TEST_CASE("Stab: Determinismus (wiederholt und auf Threads Bit fuer Bit), Abdruck je Instrument, Rechenzeit")
{
    const std::vector<StabSpiel>& a = alle();
    std::vector<std::vector<float>> einzeln(a.size()), parallel(a.size());
    const auto ton = [&](std::size_t i, std::vector<float>& aus) {
        const StabSpiel& s = a[i];
        StabAnschlag n = anschlag((s.noteMin + s.noteMax) / 2, -12);
        if (s.motor) n.motor = {{0.0, 3.0}};
        rechneStabTon(s, n, aus, stabNachklang(s, n, 48000 * 3));
    };
    for (std::size_t i = 0; i < a.size(); ++i) ton(i, einzeln[i]);
    parallelJe(a.size(), [&](std::size_t i) { ton(i, parallel[i]); }, 4);
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        CAPTURE(a[i].name);
        CHECK(einzeln[i] == parallel[i]);
        const std::string ist = abdruck(einzeln[i]);
        CHECK(test::erwarteterAbdruck("stab_" + a[i].name + "_mitte_a-12", ist) == ist);
        CHECK(spitze(einzeln[i]) > 0.0);
        CHECK(std::isfinite(rms(einzeln[i])));
    }
    // Rechenzeit: tiefster Ton, 2 s
    for (const StabSpiel& s : a)
    {
        StabAnschlag n = anschlag(s.noteMin, 0);
        if (s.motor) n.motor = {{0.0, 3.0}};
        std::vector<float> aus;
        const double anteil = echtzeitAnteil([&] { rechneStabTon(s, n, aus, 96000); }, 2.0);
        std::printf("Rechenzeit %-15s tiefster Ton %3d: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", s.name.c_str(), s.noteMin,
                    100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
        CAPTURE(s.name);
        CHECK(anteil < test::echtzeitGrenzeHier());
    }
}
