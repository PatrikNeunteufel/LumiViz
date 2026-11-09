/**
 * @file test_blech.cpp
 * @brief Klasse 7 Lippe und Rohr: die Schleife gegen die Uebertragungsmatrix (linear), die gestimmte Posaune trifft
 *        jede Note, Trompete mit Ventilen, Zug in der Kette, Schmettern nach Myers et al. 2012, Alphorn gegen Marcinnò et al. 2026,
 *        Vibrato, Growl, Daempfer, Gerueste ohne Bohrung, Determinismus, Abdruck, Rechenzeit
 */
#include "doctest.h"

#include "gemeinsam/Fft.hpp"
#include "gemeinsam/Rohrschleife.hpp"
#include "klassen/07_lippe_rohr/Blech.hpp"
#include "messung/Bausteine.hpp"
#include "werkzeuge/TestHilfe.hpp"

#include <cmath>
#include <cstdio>
#include <numbers>

using namespace ic;
using namespace ic::blech;

namespace
{

constexpr double kPi = std::numbers::pi;

double hzVon(int n)
{
    return 440.0 * std::exp2((n - 69) / 12.0);
}

const BlechSpiel& posaune()
{
    static const BlechSpiel p = baueVorgabePosaune();
    return p;
}

const BlechSpiel& trompete()
{
    static const BlechSpiel t = baueVorgabeTrompete();
    return t;
}

BlechAnschlag ton(int note, double e, double haltenS = 0.6)
{
    BlechAnschlag a;
    a.kette = {{0.0, note, hzVon(note)}};
    a.expression = e;
    a.haltenS = haltenS;
    return a;
}

double schwerpunkt(const std::vector<float>& t, std::size_t ab, std::size_t n)
{
    std::vector<double> x(n);
    for (std::size_t i = 0; i < n; ++i) x[i] = t[ab + i] * (0.5 - 0.5 * std::cos(2.0 * kPi * i / n));
    std::vector<std::complex<double>> s(n / 2 + 1);
    fft().vorwaerts(x, s);
    double a = 0.0, b = 0.0;
    for (std::size_t k = 1; k < s.size(); ++k)
    {
        a += k * std::norm(s[k]);
        b += std::norm(s[k]);
    }
    return a / b * 48000.0 / n;
}

}  // namespace

TEST_CASE("Rohrschleife linear: der Kessel vor der Bohrung trifft die Uebertragungsmatrix (Z mit Kessel)")
{
    const BlechSpiel& p = posaune();
    const Luft luft;
    Rohrschleife s(schleifenFilter(p.rest, p.zylinderRadiusM, p.zylinderGrundM, 48000, luft, 4096), p.kesselM3, p.zylinderGrundM, 48000, luft);
    s.setzeNichtlinear(0.0);
    std::vector<double> pk(1 << 15);
    for (std::size_t i = 0; i < pk.size(); ++i)
    {
        pk[i] = s.kesselDruck();
        s.schritt(i == 0 ? 48000.0 : 0.0);  // ein Flussstoss 1 m³/s·Takt
    }
    // die Spitzen von |Z| aus der Impulsantwort gegen die der Uebertragungsmatrix mit Kessel (Abweichung in Cent)
    const std::vector<double> soll = registerSpitzen(p, 0.0, 600.0);
    REQUIRE(soll.size() >= 6);
    for (std::size_t n = 1; n < 6; ++n)
    {
        double beste = 0.0, wert = 0.0;
        for (double f = soll[n] * 0.95; f <= soll[n] * 1.05; f += 0.05)
        {
            std::complex<double> z(0.0, 0.0);
            for (std::size_t i = 0; i < pk.size(); ++i) z += pk[i] * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * i / 48000.0));
            if (std::abs(z) > wert)
            {
                wert = std::abs(z);
                beste = f;
            }
        }
        CAPTURE(n);
        CHECK(std::abs(1200.0 * std::log2(beste / soll[n])) < 5.0);
    }
}

TEST_CASE("Posaune: die gestimmte Vorgabe trifft jede vierte Note bei pp, mf, ff auf ±10 Cent und schwingt")
{
    const BlechSpiel& p = posaune();
    for (int note = p.noteMin + 1; note <= p.noteMax - 4; note += 4)
        for (const double e : {0.0, 0.5, 1.0})
        {
            BlechMessung m;
            std::vector<float> t;
            rechneBlechTon(p, ton(note, e), t, static_cast<std::int64_t>(0.7 * 48000), &m);
            CAPTURE(note);
            CAPTURE(e);
            REQUIRE(m.schwingt);
            CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(note))) < 10.0);
        }
}

TEST_CASE("Trompete (Soares et al. 2025): jede dritte Note bei pp, mf, ff — mindestens 90 % auf ±10 Cent; Ventilwechsel in der Kette")
{
    const BlechSpiel& tr = trompete();
    REQUIRE(tr.spielbar);
    int getroffen = 0, gesamt = 0;
    for (int note = tr.noteMin; note <= tr.noteMax; note += 3)
        for (const double e : {0.0, 0.5, 1.0})
        {
            BlechMessung m;
            std::vector<float> t;
            rechneBlechTon(tr, ton(note, e), t, static_cast<std::int64_t>(0.7 * 48000), &m);
            ++gesamt;
            const double c = m.schwingt ? 1200.0 * std::log2(m.spielHz / hzVon(note)) : 1e9;
            if (std::abs(c) < 10.0)
                ++getroffen;
            else
                std::printf("WARN Trompete n%d e%.1f: %+.0f Cent\n", note, e, c);
        }
    std::printf("Trompete: %d von %d Pruefpunkten auf ±10 Cent\n", getroffen, gesamt);
    CHECK(getroffen * 10 >= gesamt * 9);
    // B♭4 (offen) → A4 (Ventil 2): der Griff wechselt sofort, die zweite Note sitzt
    BlechAnschlag a = ton(70, 0.5, 1.2);
    a.kette.push_back({0.6, 69, hzVon(69)});
    BlechMessung m;
    std::vector<float> t;
    rechneBlechTon(tr, a, t, static_cast<std::int64_t>(1.3 * 48000), &m);
    REQUIRE(m.schwingt);
    CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(69))) < 15.0);
}

TEST_CASE("Posaune: Kette verbundener Noten — der Zug bewegt sich, die zweite Note sitzt; lauter mit der Staerke")
{
    const BlechSpiel& p = posaune();
    BlechAnschlag a = ton(58, 0.5, 1.2);
    a.kette.push_back({0.6, 56, hzVon(56)});  // B♭3 → A♭3: eine Lage weiter
    BlechMessung m;
    std::vector<float> t;
    rechneBlechTon(p, a, t, static_cast<std::int64_t>(1.3 * 48000), &m);
    REQUIRE(m.schwingt);
    CHECK(std::abs(1200.0 * std::log2(m.spielHz / hzVon(56))) < 15.0);
    std::vector<float> leise, laut;
    rechneBlechTon(p, ton(58, 0.0), leise, 28800);
    rechneBlechTon(p, ton(58, 1.0), laut, 28800);
    CHECK(rms(laut) > 2.0 * rms(leise));
}

TEST_CASE("Posaune gegen Myers et al. 2012: das Schmettern hebt den Schwerpunkt am Schallstueck mehr als im Mundstueck")
{
    // Myers S. 681 (nachgelesen): im Crescendo steigt der Schwerpunkt vor der Stuerze „from 1.5 to 4.0“ (×2,7), im
    // Mundstueck nur „from 1.2 to 1.6“ (×1,3). Geprueft: am Schallstueck steigt er staerker als im Kessel, und ohne
    // die nichtlineare Ausbreitung (β = 0) weniger als mit
    const BlechSpiel& p = posaune();
    const auto anstieg = [&](const BlechSpiel& s, bool kessel) {
        std::vector<float> pp, ff;
        BlechAnschlag a = ton(58, 0.0);
        a.kesselAusgeben = kessel;
        rechneBlechTon(s, a, pp, 28800);
        a.expression = 1.0;
        rechneBlechTon(s, a, ff, 28800);
        return schwerpunkt(ff, 14400, 8192) / schwerpunkt(pp, 14400, 8192);
    };
    const double schall = anstieg(p, false);
    const double kessel = anstieg(p, true);
    BlechSpiel linear = p;
    linear.beta = 0.0;
    const double ohne = anstieg(linear, false);
    std::printf("Schmettern: Schwerpunkt ff/pp am Schallstueck %.2f (linear %.2f), im Kessel %.2f (Myers 2,7 bzw. 1,3)\n", schall, ohne,
                kessel);
    CHECK(schall > kessel);
    CHECK(schall > ohne);
}

TEST_CASE("Alphorn gegen Marcinnò et al. 2026: Spitzen der Eingangsimpedanz gegen die Zielskala 92/139/185 … Hz")
{
    const BlechSpiel a = baueVorgabeAlphorn(false);
    const std::vector<double> z = registerSpitzen(a, 0.0, 600.0);
    REQUIRE(z.size() >= 6);
    // die Arbeit (Tab. 1 S. 10, nur Bericht): FEM-Spitzen +64 … +112 Cent ueber dem Ziel (verlustfrei). Hier fehlen R1,
    // Mundstueck und Becher (offen.md) — die Bohrung ist rund 0,4 m kuerzer als ein Fis-Horn (347 cm, Hersteller, nur
    // Bericht), die Reihe liegt deshalb rund 3 Halbtoene hoch und ist ohne Mundstueck nicht harmonisch (bis +110 Cent
    // gegen n/2). Das ist ein Befund (offen.md), keine Pruefung: gemeldet als WARN; geprueft nur die Groessenordnung
    // (Spitze 2 innerhalb von ±300 Cent um das Ziel)
    const double ziel[] = {92.0, 139.0, 185.0, 231.0, 278.0};
    for (int i = 0; i < 5; ++i)
    {
        const double c = 1200.0 * std::log2(z[static_cast<std::size_t>(i + 1)] / ziel[i]);
        const double reihe = 1200.0 * std::log2(z[static_cast<std::size_t>(i + 1)] / z[1] / ((i + 2) / 2.0));
        std::printf("Alphorn Spitze %d: %.1f Hz gegen %.0f Hz: %+.0f Cent; gegen die Reihe %+.0f Cent\n", i + 2,
                    z[static_cast<std::size_t>(i + 1)], ziel[i], c, reihe);
        WARN(std::abs(reihe) < 60.0);
        if (i == 0) CHECK(std::abs(c) < 300.0);
    }
}

TEST_CASE("Posaune: Vibrato der Lippe, Growl mit Kombinationstoenen, Daempfer dunkler; Gerueste ohne Bohrung still")
{
    const BlechSpiel& p = posaune();
    BlechAnschlag v = ton(65, 0.5, 1.5);
    v.vibratoCent = 30.0;
    v.vibratoHz = 5.0;
    std::vector<float> mitV, ohneV;
    rechneBlechTon(p, v, mitV, 72000);
    rechneBlechTon(p, ton(65, 0.5, 1.5), ohneV, 72000);
    CHECK(mitV != ohneV);
    std::vector<double> x(mitV.begin() + 24000, mitV.begin() + 72000);
    const messung::Tonhoehe th = messung::tonhoehe(x, 48000, 200.0, 600.0);
    double lo = 1e9, hi = 0.0;
    for (const double f : th.hz)
        if (f > 0.0)
        {
            lo = std::min(lo, f);
            hi = std::max(hi, f);
        }
    CHECK(1200.0 * std::log2(hi / lo) > 5.0);  // die Tonhoehe schwankt (das Verhaeltnis zur Lippe kommt aus dem Modell)
    BlechAnschlag g = ton(53, 0.7, 0.8);
    g.growl = true;
    std::vector<float> mitG, ohneG;
    rechneBlechTon(p, g, mitG, 38400);
    rechneBlechTon(p, ton(53, 0.7, 0.8), ohneG, 38400);
    CHECK(mitG != ohneG);
    BlechAnschlag d = ton(58, 0.7);
    d.daempfer = 1;  // Cup
    std::vector<float> mitD, ohneD;
    rechneBlechTon(p, d, mitD, 28800);
    rechneBlechTon(p, ton(58, 0.7), ohneD, 28800);
    CHECK(rms(mitD) < rms(ohneD));
    for (const BlechSpiel& s : {baueVorgabeTuba(), baueVorgabeHorn(), baueVorgabeDidgeridoo()})
    {
        CAPTURE(s.name);
        CHECK_FALSE(s.spielbar);
        std::vector<float> t;
        rechneBlechTon(s, ton((s.noteMin + s.noteMax) / 2, 0.5), t, 4800);
        CHECK(spitze(t) == 0.0);
    }
}

TEST_CASE("Posaune: Wah-Wah am Harmon (T3b a) — die Hand schliesst und oeffnet in der Note, der Pegel folgt")
{
    const BlechSpiel& p = posaune();
    REQUIRE(p.daempfer.size() >= 3);
    REQUIRE(p.daempfer[2].restZu > 0.0);
    BlechAnschlag a = ton(58, 0.7, 1.2);
    a.daempfer = 2;  // Harmon
    a.wah = {{0.0, 0.0}, {0.55, 0.0}, {0.6, 1.0}};
    std::vector<float> mit, ohne;
    rechneBlechTon(p, a, mit, static_cast<std::int64_t>(1.2 * 48000));
    BlechAnschlag b = a;
    b.wah.clear();
    rechneBlechTon(p, b, ohne, static_cast<std::int64_t>(1.2 * 48000));
    const auto pegel = [](const std::vector<float>& t, double von, double bis) {
        double e = 0.0;
        const auto i0 = static_cast<std::size_t>(von * 48000), i1 = static_cast<std::size_t>(bis * 48000);
        for (std::size_t i = i0; i < i1; ++i) e += double(t[i]) * t[i];
        return 10.0 * std::log10(e / static_cast<double>(i1 - i0) + 1e-30);
    };
    const double zu = pegel(mit, 0.35, 0.55), offen = pegel(mit, 0.85, 1.05), stets = pegel(ohne, 0.85, 1.05);
    std::printf("Posaune Harmon Wah-Wah: zu %.1f dB, offen %.1f dB (ohne Wah %.1f dB)\n", zu, offen, stets);
    CHECK(offen - zu > 3.0);
    CHECK(std::abs(offen - stets) < 1.5);  // offen ist der Harmon ohne Hand
}

TEST_CASE("Blech: Determinismus, Abdruck, Rechenzeit")
{
    const BlechSpiel& p = posaune();
    const BlechSpiel al = baueVorgabeAlphorn();
    std::vector<std::vector<float>> a(2), b(2);
    const auto rechne = [&](std::size_t i, std::vector<float>& t) {
        rechneBlechTon(i == 0 ? p : al, ton(i == 0 ? 58 : 57, 0.5), t, 28800);
    };
    for (std::size_t i = 0; i < 2; ++i) rechne(i, a[i]);
    parallelJe(2, [&](std::size_t i) { rechne(i, b[i]); }, 2);
    for (std::size_t i = 0; i < 2; ++i)
    {
        CHECK(a[i] == b[i]);
        const std::string ist = abdruck(a[i]);
        CHECK(test::erwarteterAbdruck(i == 0 ? "blech_Posaune_m58_mf" : "blech_Alphorn_m57_mf", ist) == ist);
    }
    std::vector<float> tr;
    rechneBlechTon(trompete(), ton(70, 0.5), tr, 28800);
    const std::string istTr = abdruck(tr);
    CHECK(test::erwarteterAbdruck("blech_Trompete_m70_mf", istTr) == istTr);
    std::vector<float> t;
    const double anteil = echtzeitAnteil([&] { rechneBlechTon(p, ton(p.noteMin, 0.5, 2.0), t, 96000); }, 2.0);
    std::printf("Rechenzeit Posaune tiefster Ton: %.2f %% der Echtzeit (Grenze hier %.2f %%)\n", 100.0 * anteil, 100.0 * test::echtzeitGrenzeHier());
    CHECK(anteil < test::echtzeitGrenzeHier());
}
