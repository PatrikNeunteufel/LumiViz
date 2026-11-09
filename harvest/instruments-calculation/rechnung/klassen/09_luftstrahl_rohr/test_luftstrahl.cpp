/**
 * @file test_luftstrahl.cpp
 * @brief Klasse 9 Luftstrahl und Rohr: offener Zylinder (Spitzen der Admittanz 1 : 2 : 3), die Labialpfeife trifft
 *        jede Note, Schweller und Tremulant wirken, die Blockfloete trifft ihre Noten und steigt mit dem Druck (Fletcher &
 *        Douglas S. 767), ueberblasen in der zweiten Oktave, Shakuhachi als Befund, Gerueste still, Determinismus,
 *        Abdruck, Rechenzeit
 */
#include "doctest.h"

#include "klassen/09_luftstrahl_rohr/Luftstrahl.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <cstdio>

using namespace ic;
using namespace ic::luftstrahl;

namespace
{

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

StrahlAnschlag ton(int note, double e, double haltenS = 0.8)
{
    StrahlAnschlag a;
    a.kette = {{0.0, note, hzVon(note)}};
    a.expression = e;
    a.haltenS = haltenS;
    return a;
}

double centBei(const StrahlSpiel& s, const StrahlAnschlag& a, bool* schwingt = nullptr)
{
    StrahlMessung m;
    std::vector<float> t;
    rechneStrahlTon(s, a, t, static_cast<std::int64_t>(a.haltenS * a.rate), &m);
    if (schwingt != nullptr) *schwingt = m.schwingt;
    return m.schwingt ? 1200.0 * std::log2(m.spielHz / a.kette.front().hz) : 1e9;
}

const StrahlSpiel& pfeife()
{
    static const StrahlSpiel p = baueVorgabeLabialpfeife();
    return p;
}

const StrahlSpiel& blockfloete()
{
    static const StrahlSpiel b = baueVorgabeBlockfloete();
    return b;
}

}  // namespace

TEST_CASE("Luftstrahl: das offene Rohr hat Spitzen der Admittanz 1 : 2 : 3")
{
    StrahlSpiel s = baueVorgabeBlockfloete(false);
    s.laengeJeNote = kurveKonstant(0.3, Belegkraft::Offen, "", 0, 127);
    const std::vector<ImpedanzMode> m = rohrModen(s, 72);
    REQUIRE(m.size() >= 3);
    CHECK(m[0].hz == doctest::Approx(343.0 / (2.0 * (0.3 + 2 * 0.61 * 0.0095))).epsilon(0.05));
    CHECK(m[1].hz / m[0].hz == doctest::Approx(2.0).epsilon(0.03));
    CHECK(m[2].hz / m[0].hz == doctest::Approx(3.0).epsilon(0.03));
}

TEST_CASE("Labialpfeife: jede vierte Note trifft auf ±10 Cent; der Aufschnitt waechst mit der Mensur")
{
    const StrahlSpiel& p = pfeife();
    for (int note = p.noteMin; note <= p.noteMax; note += 4)
    {
        CAPTURE(note);
        CHECK(std::abs(centBei(p, ton(note, 0.5))) < 10.0);
    }
    REQUIRE(p.aufschnittJeNote.punkte.size() >= 2);
    CHECK(wertAn(p.aufschnittJeNote, 36, Skala::Logarithmisch) > wertAn(p.aufschnittJeNote, 72, Skala::Logarithmisch));
}

TEST_CASE("Labialpfeife: der Schweller macht lauter, der Tremulant schwankt mit ~5 Hz, der Ton haelt die Hoehe")
{
    const StrahlSpiel& p = pfeife();
    std::vector<float> leise, laut, trem;
    rechneStrahlTon(p, ton(60, 0.0), leise, 38400);
    rechneStrahlTon(p, ton(60, 1.0), laut, 38400);
    CHECK(rms(laut) > 2.0 * rms(leise));
    StrahlAnschlag a = ton(60, 1.0, 1.6);
    a.vibratoCent = 100.0;
    a.vibratoHz = 5.0;
    rechneStrahlTon(p, a, trem, 76800);
    // die Huelle in 20-ms-Stuecken ab 0,4 s: Schwankung deutlich ueber der ohne Tremulant
    const auto schwankung = [](const std::vector<float>& t) {
        double lo = 1e300, hi = 0.0;
        for (std::size_t i = 19200; i + 960 <= std::min<std::size_t>(t.size(), 72000); i += 960)
        {
            double e = 0.0;
            for (std::size_t j = 0; j < 960; ++j) e += double(t[i + j]) * t[i + j];
            lo = std::min(lo, e);
            hi = std::max(hi, e);
        }
        return hi / std::max(lo, 1e-300);
    };
    std::vector<float> ruhig;
    rechneStrahlTon(p, ton(60, 1.0, 1.6), ruhig, 76800);
    std::printf("Labialpfeife C4: Schweller %.1f dB, Tremulant Huelle %.1f dB (ohne %.1f dB)\n", dB(rms(laut) / rms(leise)),
                10.0 * std::log10(schwankung(trem)), 10.0 * std::log10(schwankung(ruhig)));
    // in dB der Huelle: deutlich (≥ 2 dB) und ein Vielfaches der ohne Tremulant
    CHECK(10.0 * std::log10(schwankung(trem)) > 2.0);
    CHECK(10.0 * std::log10(schwankung(trem)) > 4.0 * 10.0 * std::log10(schwankung(ruhig)));
}

TEST_CASE("Blockfloete: die Noten mit Fenster treffen bei mf auf ±10 Cent und steigen mit dem Druck; die zweite Oktave ueberblasen")
{
    const StrahlSpiel& b = blockfloete();
    int getroffen = 0, gesamt = 0, ueberblasen = 0;
    for (int note = b.noteMin; note <= b.noteMax; ++note)
    {
        ++gesamt;
        if (b.registerJeNote.size() > static_cast<std::size_t>(note - b.noteMin) && b.registerJeNote[static_cast<std::size_t>(note - b.noteMin)] == 2)
            ++ueberblasen;
        const double c = centBei(b, ton(note, 0.5));
        if (std::abs(c) < 10.0) ++getroffen;
        else std::printf("WARN Blockfloete n%d: mf %+.0f Cent\n", note, c);
    }
    std::printf("Blockfloete: %d von %d Noten auf ±10 Cent, %d ueberblasen\n", getroffen, gesamt, ueberblasen);
    CHECK(getroffen >= gesamt * 3 / 4);
    // mehr Druck → hoeher (Fletcher & Douglas S. 767, nachgelesen)
    const double pp = centBei(b, ton(74, 0.0)), ff = centBei(b, ton(74, 1.0));
    CHECK(ff > pp);
    CHECK(ff - pp < 100.0);
}

TEST_CASE("Shakuhachi spielt ihren Grundton; Querfloete und Altfloete ohne Bohrung still")
{
    const StrahlSpiel sh = baueVorgabeShakuhachi();
    int getroffen = 0;
    for (int note = sh.noteMin; note <= sh.noteMax; ++note)
        if (std::abs(centBei(sh, ton(note, 0.5))) < 10.0) ++getroffen;
    std::printf("Shakuhachi: %d von %d Noten auf ±10 Cent (Befund, Mund offen)\n", getroffen, sh.noteMax - sh.noteMin + 1);
    CHECK(std::abs(centBei(sh, ton(sh.noteMin, 0.5))) < 10.0);
    for (const StrahlSpiel& s : {baueVorgabeQuerfloete(), baueVorgabeAltfloete()})
    {
        CAPTURE(s.name);
        CHECK_FALSE(s.spielbar);
        std::vector<float> t;
        rechneStrahlTon(s, ton((s.noteMin + s.noteMax) / 2, 0.5), t, 4800);
        CHECK(spitze(t) == 0.0);
    }
}

TEST_CASE("Luftstrahl: Determinismus, Abdruck, Rechenzeit")
{
    const StrahlSpiel& p = pfeife();
    std::vector<float> a;
    rechneStrahlTon(p, ton(60, 0.5), a, 28800);
    std::vector<std::vector<float>> q(2);
    parallelJe(2, [&](std::size_t i) { rechneStrahlTon(p, ton(60, 0.5), q[i], 28800); }, 2);
    CHECK(a == q[0]);
    CHECK(a == q[1]);
    const std::string ist = abdruck(a);
    CHECK(test::erwarteterAbdruck("luftstrahl_Labialpfeife_m60_mf", ist) == ist);
    const std::string istB = [&] {
        std::vector<float> t;
        rechneStrahlTon(blockfloete(), ton(74, 0.5), t, 28800);
        return abdruck(t);
    }();
    CHECK(test::erwarteterAbdruck("luftstrahl_Blockfloete_m74_mf", istB) == istB);
    std::vector<float> t;
    const double anteil = echtzeitAnteil([&] { rechneStrahlTon(p, ton(p.noteMin, 0.5, 2.0), t, 96000); }, 2.0);
    std::printf("Rechenzeit Labialpfeife C2: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
    CHECK(anteil < test::echtzeitGrenzeHier());
}
