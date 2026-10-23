/**
 ****************************************************************************************
 * @file   test_LoopKoerperStatistik.cpp
 * @brief  Loop, Stufe 31c-5c: der statistische Teil des Koerpers oberhalb der gerechneten
 *         Moden (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S146–S148: KB1 a,
 *         PL31c-5 a, ST1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung, ohne Dateien.
 *
 * - Der Zufall: derselbe Samen gibt dieselbe Folge, eine feste Folge als Bezug (auf jedem
 *   Compiler gleich), Rayleigh mit Mittel 1 und Varianz 4/π − 1, normal mit 0 und 1.
 * - Die Ziehung: Zahl der Moden wie ∫n df, Abstaende nach Rayleigh, Formwerte mit der
 *   Streuung je Stegpunkt, Gewichte mit festem Betrag und gestreuter Phase, Daempfung nach
 *   dem Gesetz; zwei Ziehungen mit demselben Samen gleich, mit anderem verschieden.
 * - Die Schaetzung aus gerechneten Moden: eine Dichte mit bekanntem Anstieg, Streuung und
 *   Betrag kommen zurueck; eine fallende Dichte wird waagrecht; zu wenige Moden: leer.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Statistik.hpp"

#include <cmath>
#include <complex>
#include <numbers>

namespace kp = lumi::koerper;

namespace
{

kp::StatistischerTeil teil(double dichte, double anstieg)
{
    kp::StatistischerTeil st;
    st.vonHz = 5000.0;
    st.bisHz = 20000.0;
    st.dichte = dichte;
    st.anstieg = anstieg;
    st.stegStreuung = {0.5, 2.0, 0.0};
    st.hoerBetrag = {0.01, 0.003};
    st.samen = 4711;
    return st;
}

/// Gerechnete Moden mit bekannter Dichte n(f) = n0 + s (f − f0) zwischen f0 und f1, Formwerte
/// abwechselnd ±a, Gewichte vom Betrag g.
std::vector<kp::ModeImKoerper> gerechnet(double f0, double f1, double n0, double s, double a, double g)
{
    std::vector<kp::ModeImKoerper> moden;
    for (double f = f0; f < f1;)
    {
        kp::ModeImKoerper m;
        m.frequenzHz = f;
        m.amStegpunkt = {moden.size() % 2 ? a : -a};
        m.abstrahlung = {std::polar(g, 0.3 * static_cast<double>(moden.size()))};
        moden.push_back(m);
        f += 1.0 / (n0 + s * (f - f0));
    }
    return moden;
}

}  // namespace

TEST_CASE("Loop Koerper 31c-5c Zufall")
{
    kp::Zufall a(1), b(1), c(2);
    for (int i = 0; i < 100; ++i)
    {
        const std::uint64_t x = a.naechste();
        CHECK(x == b.naechste());
        CHECK(x != c.naechste());
    }
    // SplitMix64 mit Samen 0: die ersten Zahlen (Bezug — aendert sich das, aendern sich alle Koerper)
    kp::Zufall null(0);
    CHECK(null.naechste() == 0xE220A8397B1DCDAFull);
    CHECK(null.naechste() == 0x6E789E6AA1B965F4ull);

    kp::Zufall z(99);
    const int n = 200000;
    double s1 = 0.0, s2 = 0.0, g1 = 0.0, g2 = 0.0, u1 = 0.0;
    int ausserhalb = 0;
    for (int i = 0; i < n; ++i)
    {
        const double r = z.rayleigh();
        s1 += r;
        s2 += r * r;
        const double x = z.normal();
        g1 += x;
        g2 += x * x;
        const double u = z.gleich();
        ausserhalb += (u > 0.0 && u < 1.0) ? 0 : 1;
        u1 += u;
    }
    CHECK(ausserhalb == 0);
    const double mittel = s1 / n, varianz = s2 / n - mittel * mittel;
    CHECK(mittel == doctest::Approx(1.0).epsilon(0.01));
    CHECK(varianz == doctest::Approx(4.0 / std::numbers::pi - 1.0).epsilon(0.03));
    CHECK(std::abs(g1 / n) < 0.01);
    CHECK(g2 / n == doctest::Approx(1.0).epsilon(0.01));
    CHECK(u1 / n == doctest::Approx(0.5).epsilon(0.01));
}

TEST_CASE("Loop Koerper 31c-5c Ziehung")
{
    const kp::StatistischerTeil st = teil(0.23, 1.6e-5);  // wie der Steinway am oberen Ende
    const auto moden = kp::ziehe(st);
    // so viele Moden wie ∫n df = n0 Δf + s Δf²/2
    const double df = st.bisHz - st.vonHz;
    const double soll = st.dichte * df + st.anstieg * df * df / 2.0;
    MESSAGE(moden.size(), " Moden gezogen, erwartet ", soll);
    CHECK(static_cast<double>(moden.size()) == doctest::Approx(soll).epsilon(0.03));
    REQUIRE(moden.size() > 100);
    CHECK(moden.front().frequenzHz > st.vonHz);
    CHECK(moden.back().frequenzHz < st.bisHz);

    // Abstaende auf die oertliche Dichte normiert: Rayleigh (Mittel 1, Varianz 4/π − 1)
    double s1 = 0.0, s2 = 0.0;
    for (std::size_t i = 1; i < moden.size(); ++i)
    {
        const double x = (moden[i].frequenzHz - moden[i - 1].frequenzHz) * st.dichteBei(moden[i - 1].frequenzHz);
        s1 += x;
        s2 += x * x;
    }
    const double n = static_cast<double>(moden.size() - 1);
    CHECK(s1 / n == doctest::Approx(1.0).epsilon(0.03));
    CHECK(s2 / n - (s1 / n) * (s1 / n) == doctest::Approx(4.0 / std::numbers::pi - 1.0).epsilon(0.1));

    // Formwerte: Streuung je Stegpunkt, Mittel 0; Gewichte: fester Betrag, Phase gestreut
    std::vector<double> q(3, 0.0), m(3, 0.0);
    std::complex<double> phase{0.0, 0.0};
    int falsch = 0;  // Groesse, Betrag oder Daempfung daneben
    for (const kp::ModeImKoerper& mode : moden)
    {
        if (mode.amStegpunkt.size() != 3 || mode.abstrahlung.size() != 2)
        {
            ++falsch;
            continue;
        }
        for (std::size_t j = 0; j < 3; ++j)
        {
            q[j] += mode.amStegpunkt[j] * mode.amStegpunkt[j];
            m[j] += mode.amStegpunkt[j];
        }
        falsch += std::abs(std::abs(mode.abstrahlung[0]) - 0.01) > 1e-12 ? 1 : 0;
        falsch += std::abs(std::abs(mode.abstrahlung[1]) - 0.003) > 1e-12 ? 1 : 0;
        falsch += std::abs(mode.verlustfaktor - st.daempfung.verlustfaktor(mode.frequenzHz)) > 1e-15 ? 1 : 0;
        phase += mode.abstrahlung[0] / std::abs(mode.abstrahlung[0]);
    }
    CHECK(falsch == 0);
    const double nm = static_cast<double>(moden.size());
    CHECK(std::sqrt(q[0] / nm) == doctest::Approx(0.5).epsilon(0.05));
    CHECK(std::sqrt(q[1] / nm) == doctest::Approx(2.0).epsilon(0.05));
    CHECK(q[2] == 0.0);
    CHECK(std::abs(m[0] / nm) < 0.05 * 0.5);
    CHECK(std::abs(phase) / nm < 0.05);

    // derselbe Samen: dieselben Moden; ein anderer: andere
    const auto nochmal = kp::ziehe(st);
    REQUIRE(nochmal.size() == moden.size());
    int anders0 = 0;
    for (std::size_t i = 0; i < moden.size(); ++i)
    {
        anders0 += (nochmal[i].frequenzHz == moden[i].frequenzHz && nochmal[i].amStegpunkt == moden[i].amStegpunkt &&
                    nochmal[i].abstrahlung == moden[i].abstrahlung)
                       ? 0
                       : 1;
    }
    CHECK(anders0 == 0);
    kp::StatistischerTeil anders = st;
    anders.samen = 4712;
    CHECK(kp::ziehe(anders).front().frequenzHz != moden.front().frequenzHz);

    // leer: keine Dichte, kein Bereich
    CHECK(kp::ziehe(teil(0.0, 0.0)).empty());
    kp::StatistischerTeil verkehrt = st;
    verkehrt.bisHz = st.vonHz;
    CHECK(kp::ziehe(verkehrt).empty());
}

TEST_CASE("Loop Koerper 31c-5c Schaetzung")
{
    // gerechnete Moden 0 … 5 kHz mit n(f) = 0,1 + 2·10⁻⁵ (f − 2500) im Band, Formwerte ±0,8, |G| = 0,02
    const auto vorher = gerechnet(0.0, 2500.0, 0.06, 0.0, 0.3, 0.05);
    const auto band = gerechnet(2500.0, 5000.0, 0.1, 2.0e-5, 0.8, 0.02);
    std::vector<kp::ModeImKoerper> alle = vorher;
    alle.insert(alle.end(), band.begin(), band.end());
    const kp::StatistischerTeil st = kp::schaetze(alle, 2500.0, 5000.0, 5000.0, 20000.0, kp::Daempfung{}, 1);
    CHECK(st.bandModen == static_cast<int>(band.size()));
    CHECK(st.anstieg == doctest::Approx(2.0e-5).epsilon(0.1));  // die Zaehlung in Stuecken rundet auf ganze Moden
    CHECK(st.dichte == doctest::Approx(0.1 + 2.0e-5 * 2500.0).epsilon(0.02));
    REQUIRE(st.stegStreuung.size() == 1);
    CHECK(st.stegStreuung[0] == doctest::Approx(0.8));
    REQUIRE(st.hoerBetrag.size() == 1);
    CHECK(st.hoerBetrag[0] == doctest::Approx(0.02));
    CHECK(st.vonHz == 5000.0);
    CHECK(st.bisHz == 20000.0);

    // eine fallende Dichte wird waagrecht (die Dichte einer Platte faellt nicht)
    const auto fallend = gerechnet(2500.0, 5000.0, 0.2, -1.0e-5, 0.8, 0.02);
    const kp::StatistischerTeil flach = kp::schaetze(fallend, 2500.0, 5000.0, 5000.0, 20000.0, kp::Daempfung{}, 1);
    CHECK(flach.anstieg == 0.0);
    CHECK(flach.dichte == doctest::Approx(static_cast<double>(fallend.size()) / 2500.0).epsilon(0.02));

    // zu wenige Moden im Band: leer
    const auto wenige = gerechnet(2500.0, 2550.0, 0.1, 0.0, 0.8, 0.02);
    CHECK(kp::schaetze(wenige, 2500.0, 5000.0, 5000.0, 20000.0, kp::Daempfung{}, 1).dichte == 0.0);
}
