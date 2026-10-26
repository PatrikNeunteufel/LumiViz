/**
 ****************************************************************************************
 * @file   test_Teiltonmessung.cpp
 * @brief  Loop, Stufe 31d-2: die Teiltonmessung an synthetischen Signalen
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S151: UF3 a, IK1)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Jedes Signal ist aus bekannten Teiltoenen gebaut
 * (Frequenz, Amplitude, Abklingen je Stufe), mit Stille vor dem Einsatz und leisem Rauschen;
 * die Messung muss die Groessen wiederfinden:
 *
 * - Einsatz, f0 und B im Bass und im Diskant, jeder Teilton an seiner Stelle.
 * - Zwei Stufen (in dB zwei Geraden mit Knick) und eine Stufe (kein Knick erfunden).
 * - Schwebung zweier verstimmter Saiten; eine Saite allein schwebt nicht.
 * - Helligkeit zu Beginn (Pegel je Teilton) und `highDecay` wie im Modell.
 * - Das Rauschen begrenzt den gemessenen Bereich.
 * - 31d-3: Pegel zu Beginn, Terzbaender, Direktschall (ein Schlag vor dem Ton), Phantomteiltoene
 *   bei 2·f(n/2) mit bekanntem Abstand; eine Blende am Ende wird abgeschnitten; eine kurze fruehe
 *   Stufe wird erkannt; ein Teilton im Rauschen bekommt keine Abklingzeit (Befunde S152 an
 *   Salamander).
 * - 31d-7: die Tiefe der Einbrueche je Periode der Schwebung (TB1 a) gegen zwei Moden mit bekanntem
 *   Verhaeltnis.
 * - 31d-7 B (7.5.1, S155): die Kurve nach Schroeder auf dem Raster gegen die gerechnete Kurve (eine Stufe,
 *   zwei Stufen, Boden mit und ohne Abzug, gemeinsames Ende) und die Autokovarianz des Rests einer Schwebung.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Teiltonmessung.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <functional>
#include <numbers>
#include <utility>
#include <vector>

using namespace lumi::messung;

namespace
{

constexpr int kRate = 48000;

struct Komponente
{
    double frequenz = 0.0;
    double amplitude = 1.0;
    double t60Frueh = 10.0;
    double t60Spaet = 10.0;
    double knickS = 1e9;  ///< ab hier (nach dem Einsatz) faellt die Komponente mit `t60Spaet`
};

/// Die Summe der Komponenten ab `einsatzS`, davor Stille; dazu Rauschen gleichverteilt in
/// ±`rauschen` (SplitMix64, auf jedem Rechner dasselbe).
std::vector<double> erzeuge(const std::vector<Komponente>& komponenten, double dauerS, double einsatzS, double rauschen)
{
    const auto n = static_cast<std::size_t>(dauerS * kRate);
    const auto einsatz = static_cast<std::size_t>(einsatzS * kRate);
    std::vector<double> x(n, 0.0);
    double phase = 0.0;
    for (const Komponente& k : komponenten)
    {
        phase += 0.7;  // die Teiltoene starten nicht alle mit derselben Phase
        const auto knick = einsatz + static_cast<std::size_t>(std::min(k.knickS, dauerS) * kRate);
        const double w = 2.0 * std::numbers::pi * k.frequenz / kRate;
        const std::complex<double> frueh = std::exp(std::complex<double>(-std::log(1000.0) / (k.t60Frueh * kRate), w));
        const std::complex<double> spaet = std::exp(std::complex<double>(-std::log(1000.0) / (k.t60Spaet * kRate), w));
        std::complex<double> z = std::polar(k.amplitude, phase);
        for (std::size_t i = einsatz; i < n; ++i)
        {
            x[i] += z.imag();
            z *= i < knick ? frueh : spaet;
        }
    }
    std::uint64_t samen = 31;
    for (double& v : x)
    {
        samen += 0x9E3779B97F4A7C15ull;
        std::uint64_t z = samen;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        z ^= z >> 31;
        v += rauschen * (static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0);
    }
    return x;
}

double teiltonFrequenz(double f0, double b, int k)
{
    return k * f0 * std::sqrt(1.0 + b * k * k);
}

/// Je Teilton zwei Moden (N1 a): die schnelle mit `t60Frueh`, die langsame `abstandDb` leiser mit `t60Spaet` und
/// `abstandHz` hoeher — die Summe faellt in zwei Stufen und loescht sich am Uebergang fast aus.
std::vector<Komponente> zweiModen(double f0, double b, int teiltoene, double t60Frueh, double t60Spaet, double abstandDb,
                                  double abstandHz)
{
    std::vector<Komponente> k;
    for (int i = 1; i <= teiltoene && teiltonFrequenz(f0, b, i) < 0.45 * kRate; ++i)
    {
        const double f = teiltonFrequenz(f0, b, i);
        k.push_back({f, 0.3 / i, t60Frueh, t60Frueh});
        k.push_back({f + abstandHz, 0.3 / i * std::pow(10.0, abstandDb / 20.0), t60Spaet, t60Spaet});
    }
    return k;
}

/// Teiltoene 1/k laut mit derselben Abklingzeit.
std::vector<Komponente> saite(double f0, double b, int teiltoene, double t60)
{
    std::vector<Komponente> k;
    for (int i = 1; i <= teiltoene && teiltonFrequenz(f0, b, i) < 0.45 * kRate; ++i)
        k.push_back({teiltonFrequenz(f0, b, i), 0.3 / i, t60, t60});
    return k;
}

}  // namespace

TEST_CASE("Loop Teiltonmessung 31d-2 Einsatz, f0 und B")
{
    const struct
    {
        double f0;
        double b;
        int teiltoene;
    } faelle[] = {{55.0, 2e-4, 40}, {261.6, 4e-4, 30}, {1046.5, 1.5e-3, 18}};
    for (const auto& fall : faelle)
    {
        CAPTURE(fall.f0);
        const std::vector<Komponente> teile = saite(fall.f0, fall.b, fall.teiltoene, 10.0);
        const std::vector<double> ton = erzeuge(teile, 4.0, 0.2, 1e-6);
        const Ergebnis e = miss(ton, kRate, fall.f0 * std::sqrt(1.0 + fall.b), lumi::loop::fft());
        REQUIRE(e.erkannt);
        MESSAGE("f0 " << fall.f0 << ": gemessen f0 " << e.f0 << ", B " << e.b << " ± " << e.bFehler << ", "
                      << e.teiltoene.size() << " Teiltoene, Einsatz " << e.einsatzS << " s");
        CHECK(std::abs(e.einsatzS - 0.2) < 0.001);
        CHECK(e.f0 == doctest::Approx(fall.f0).epsilon(1e-4).scale(0.0));
        CHECK(e.b == doctest::Approx(fall.b).epsilon(0.01).scale(0.0));
        CHECK(e.bFehler < 0.01 * fall.b);
        CHECK(e.teiltoene.size() == teile.size());  // jeder unter 0,45·fs
        for (const Teilton& t : e.teiltoene)
        {
            CAPTURE(t.k);
            CHECK(t.frequenz == doctest::Approx(teiltonFrequenz(fall.f0, fall.b, t.k)).epsilon(1e-4).scale(0.0));
        }
    }
}

TEST_CASE("Loop Teiltonmessung 31d-3 eine fremde Spitze neben einem schwachen Teilton")
{
    // wie am gerechneten C2 (Befund S152): Teilton 2 um 30 dB leiser, 9 % darunter eine Resonanz so laut
    // wie er sonst waere — sie darf B nicht kippen
    const double f0 = 65.406;
    const double b = 1.5e-4;
    std::vector<Komponente> k = saite(f0, b, 20, 10.0);
    k[1].amplitude *= 0.03;
    k.push_back({0.908 * teiltonFrequenz(f0, b, 2), 0.15, 10.0, 10.0});
    const Ergebnis e = miss(erzeuge(k, 4.0, 0.1, 1e-6), kRate, f0 * std::sqrt(1.0 + b), lumi::loop::fft());
    REQUIRE(e.erkannt);
    MESSAGE("B gemessen " << e.b << ", " << e.teiltoene.size() << " Teiltoene");
    CHECK(e.b == doctest::Approx(b).epsilon(0.01).scale(0.0));
    CHECK(e.f0 == doctest::Approx(f0).epsilon(1e-4).scale(0.0));
    if (const Teilton* t = e.teilton(2)) CHECK(t->frequenz == doctest::Approx(teiltonFrequenz(f0, b, 2)).epsilon(1e-3).scale(0.0));
    CHECK(e.teiltoene.size() >= 18);

    // wie im rechten Kanal am gerechneten C2: eine Mode bei 0,927·f1, staerker als der schwache Teilton 1 —
    // mit Teilton 2 zusammen gaebe sie B = 0,067 und eine falsche Reihe
    std::vector<Komponente> r = saite(f0, b, 20, 10.0);
    r[0].amplitude *= 0.03;
    r.push_back({0.927 * teiltonFrequenz(f0, b, 1), 0.3, 10.0, 10.0});
    const Ergebnis rechts = miss(erzeuge(r, 4.0, 0.1, 1e-6), kRate, f0 * std::sqrt(1.0 + b), lumi::loop::fft());
    REQUIRE(rechts.erkannt);
    MESSAGE("mit Mode unter Teilton 1: B " << rechts.b << ", " << rechts.teiltoene.size() << " Teiltoene");
    CHECK(rechts.b == doctest::Approx(b).epsilon(0.01).scale(0.0));
    if (const Teilton* t = rechts.teilton(1)) CHECK(t->frequenz == doctest::Approx(teiltonFrequenz(f0, b, 1)).epsilon(1e-3).scale(0.0));
    CHECK(rechts.teiltoene.size() >= 18);
}

TEST_CASE("Loop Teiltonmessung 31d-2 zwei Stufen und eine")
{
    // zwei Stufen wie am Fluegel (Weinreich) und am Modell: je Teilton zwei Moden mit verschiedener Frequenz, die
    // schnelle laut, die langsame leise — wo sie gleich laut sind, loeschen sie sich im Takt der Schwebung fast aus
    // (Befund S152, N1 a). Die Moden liegen so weit auseinander, dass sie mehrfach je Abklingzeit schweben: sonst
    // mittelt sich ihr Mischterm nicht heraus, und es gibt keine zwei getrennten Abklingzeiten zu messen
    SUBCASE("zwei Stufen: frueh 3 s, spaet 12 s, die spaete 30 dB leiser, Ausloeschungen am Uebergang")
    {
        const Ergebnis e = miss(erzeuge(zweiModen(110.0, 1e-4, 12, 3.0, 12.0, -30.0, 2.0), 8.0, 0.1, 1e-6), kRate, 110.0,
                                lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 8; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK(t->abklingen.zweiStufen);
            CHECK(t->abklingen.t60Frueh == doctest::Approx(3.0).epsilon(0.05).scale(0.0));
            CHECK(t->abklingen.t60Spaet == doctest::Approx(12.0).epsilon(0.05).scale(0.0));
        }
    }
    SUBCASE("kurze fruehe Stufe wie an Salamander: frueh 2 s, spaet 30 s, die spaete 35 dB leiser")
    {
        const std::vector<Komponente> k = zweiModen(261.6, 3e-4, 6, 2.0, 30.0, -35.0, 1.5);
        const Ergebnis e = miss(erzeuge(k, 12.0, 0.1, 1e-6), kRate, 261.6, lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 4; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK(t->abklingen.zweiStufen);
            CHECK(t->abklingen.t60Frueh == doctest::Approx(2.0).epsilon(0.05).scale(0.0));
            CHECK(t->abklingen.t60Spaet == doctest::Approx(30.0).epsilon(0.05).scale(0.0));
        }
    }
    SUBCASE("ein Teilton, der nicht um 10 dB faellt, hat keine Abklingzeit")
    {
        // wie Teilton 1 von A0 an Salamander, der im Rauschen schwankt: leise und fast ohne Abfall
        std::vector<Komponente> k = saite(110.0, 0.0, 6, 10.0);
        k[0].amplitude = 4e-5;
        k[0].t60Frueh = k[0].t60Spaet = 100.0;
        const Ergebnis e = miss(erzeuge(k, 4.0, 0.1, 2e-5), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        if (const Teilton* t = e.teilton(1)) CHECK_FALSE(t->abklingen.erkannt);
        REQUIRE(e.teilton(2) != nullptr);
        CHECK(e.teilton(2)->abklingen.erkannt);
    }
    SUBCASE("tiefe Schwebung ohne zweite Stufe: zwei gleich laute Moden mit 4 s, 0,5 Hz auseinander")
    {
        // die Einbrueche gehen bis zur volligen Ausloeschung — die Abklingzeit bleibt 4 s, keine zweite Stufe
        std::vector<Komponente> k = saite(220.0, 0.0, 8, 4.0);
        for (const Komponente& c : saite(220.0, 0.0, 8, 4.0)) k.push_back({c.frequenz + 0.5, c.amplitude, 4.0, 4.0});
        const Ergebnis e = miss(erzeuge(k, 6.0, 0.1, 1e-6), kRate, 220.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 6; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK_FALSE(t->abklingen.zweiStufen);
            CHECK(t->abklingen.t60Frueh == doctest::Approx(4.0).epsilon(0.05).scale(0.0));
        }
    }
    SUBCASE("eine Stufe: 4 s, kein Knick")
    {
        const Ergebnis e = miss(erzeuge(saite(110.0, 1e-4, 12, 4.0), 6.0, 0.1, 1e-6), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 8; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK_FALSE(t->abklingen.zweiStufen);
            CHECK(t->abklingen.t60Frueh == doctest::Approx(4.0).epsilon(0.03).scale(0.0));
            CHECK(t->abklingen.t60Spaet == t->abklingen.t60Frueh);
        }
        for (const Teilton& t : e.teiltoene)
        {
            CAPTURE(t.k);
            CHECK_FALSE(t.abklingen.zweiStufen);
        }
    }
}

TEST_CASE("Loop Teiltonmessung 31d-2 Schwebung")
{
    // zwei Saiten, 4 Cent auseinander, die zweite halb so laut: Schwebung k·Δf
    const double f0 = 220.0;
    const double fa = f0 * std::exp2(-2.0 / 1200.0);
    const double fb = f0 * std::exp2(2.0 / 1200.0);
    std::vector<Komponente> zwei = saite(fa, 0.0, 10, 20.0);
    for (const Komponente& c : saite(fb, 0.0, 10, 20.0)) zwei.push_back({c.frequenz, 0.5 * c.amplitude, 20.0, 20.0});
    const Ergebnis e = miss(erzeuge(zwei, 6.0, 0.1, 1e-6), kRate, f0, lumi::loop::fft());
    REQUIRE(e.erkannt);
    for (int n = 1; n <= 8; ++n)
    {
        CAPTURE(n);
        const Teilton* t = e.teilton(n);
        REQUIRE(t != nullptr);
        REQUIRE(t->schwebung.erkannt);
        MESSAGE("Teilton " << n << ": Schwebung " << t->schwebung.frequenz << " Hz, Tiefe " << t->schwebung.tiefeDb << " dB");
        CHECK(t->schwebung.frequenz == doctest::Approx(n * (fb - fa)).epsilon(0.02).scale(0.0));
    }

    const Ergebnis allein = miss(erzeuge(saite(f0, 0.0, 10, 20.0), 6.0, 0.1, 1e-6), kRate, f0, lumi::loop::fft());
    REQUIRE(allein.erkannt);
    for (int n = 1; n <= 8; ++n)
    {
        CAPTURE(n);
        REQUIRE(allein.teilton(n) != nullptr);
        CHECK_FALSE(allein.teilton(n)->schwebung.erkannt);
    }
}

TEST_CASE("Loop Teiltonmessung 31d-7 Tiefe der Einbrueche")
{
    // TB1 a: je Teilton zwei Moden 0,5 Hz auseinander, die zweite r-mal so laut — die Einbrueche sind
    // 20·log10((1 + r)/(1 − r)) tief; die Grundschwingung (`tiefeDb`) saettigt dort bei rund 17 dB
    for (const double r : {0.5, 0.8, 0.95, 0.99})
    {
        CAPTURE(r);
        std::vector<Komponente> k = saite(220.0, 0.0, 8, 20.0);
        for (const Komponente& c : saite(220.0, 0.0, 8, 20.0)) k.push_back({c.frequenz + 0.5, r * c.amplitude, 20.0, 20.0});
        const Ergebnis e = miss(erzeuge(k, 8.0, 0.1, 1e-6), kRate, 220.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        const double wahr = 20.0 * std::log10((1.0 + r) / (1.0 - r));
        for (int n = 1; n <= 3; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->schwebung.erkannt);
            MESSAGE("r = " << r << ", Teilton " << n << ": wahr " << wahr << " dB, einbruchDb " << t->schwebung.einbruchDb
                           << " dB, tiefeDb " << t->schwebung.tiefeDb << " dB");
            if (r < 0.99)
                CHECK(std::abs(t->schwebung.einbruchDb - wahr) < 1.5);
            else
                CHECK(t->schwebung.einbruchDb > 35.0);  // der Schritt der Huellkurve (10 ms) trifft den Grund nicht genau
        }
    }
}

TEST_CASE("Loop Teiltonmessung 31d-7 der Bereich endet 90 dB unter der Spitze")
{
    // US6-1 a: ein Ton ohne Rauschen (wie das Modell) faellt bis weit unter jede Aufnahme; dort liegen am Modell Reste
    // der Rechnung (Befund S154 an MIDI 81: ein Boden um −150 dB, ein Sprung nach 5 s) und die Messung zaehlte
    // Einbrueche, die keine sind. Nachgebildet: T60 1,5 s, darunter je Teilton zwei gleich laute Moden 100 dB leiser,
    // 0,7 Hz auseinander, langsam abklingend (T60 15 s, damit der Bereich sie nicht als Boden abschneidet) — sie
    // schweben bis zur Ausloeschung, sobald der Ton unter sie faellt.
    std::vector<Komponente> k = saite(440.0, 0.0, 6, 1.5);
    for (const Komponente& c : saite(440.0, 0.0, 6, 1.5))
    {
        k.push_back({c.frequenz, 1e-5 * c.amplitude, 15.0, 15.0});
        k.push_back({c.frequenz + 0.7, 1e-5 * c.amplitude, 15.0, 15.0});
    }
    const std::vector<double> ton = erzeuge(k, 10.0, 0.1, 0.0);
    Einstellungen ohneGrenze;
    ohneGrenze.untergrenzeDb = 1000.0;
    const Ergebnis vorher = miss(ton, kRate, 440.0, lumi::loop::fft(), ohneGrenze);
    const Ergebnis e = miss(ton, kRate, 440.0, lumi::loop::fft());
    REQUIRE(e.erkannt);
    for (int n = 1; n <= 3; ++n)
    {
        CAPTURE(n);
        const Teilton* t = e.teilton(n);
        REQUIRE(t != nullptr);
        REQUIRE(t->abklingen.erkannt);
        const Teilton* v = vorher.teilton(n);
        REQUIRE(v != nullptr);
        MESSAGE("Teilton " << n << " ohne Grenze: bis " << v->abklingen.bisS << " s, Schwebung " << v->schwebung.erkannt << ", Einbruch "
                           << v->schwebung.einbruchDb << " dB; mit Grenze: bis " << t->abklingen.bisS << " s");
        CHECK(v->schwebung.erkannt);  // ohne Grenze der Scheinwert (gemessen S154: 46,5 dB) — der Fall sieht ihn
        CHECK(v->schwebung.einbruchDb > 30.0);
        CHECK(t->abklingen.bisS <= 2.5);  // −90 dB nach 2,25 s, dazu das Glaetten ueber ±5 Rahmen
        CHECK(t->abklingen.t60Frueh == doctest::Approx(1.5).epsilon(0.05).scale(0.0));
        CHECK((!t->schwebung.erkannt || t->schwebung.einbruchDb < 3.0));
    }
}

TEST_CASE("Loop Teiltonmessung 31d-2 Helligkeit und highDecay")
{
    SUBCASE("Pegel je Teilton zu Beginn: 1/k")
    {
        const Ergebnis e = miss(erzeuge(saite(110.0, 1e-4, 20, 10.0), 3.0, 0.1, 1e-6), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        const Teilton* erster = e.teilton(1);
        REQUIRE(erster != nullptr);
        CHECK(std::abs(erster->anfangDb - 20.0 * std::log10(0.3)) < 0.5);
        for (int n = 2; n <= 16; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            CHECK(std::abs(t->anfangDb - erster->anfangDb + 20.0 * std::log10(n)) < 0.5);
        }
    }
    SUBCASE("highDecay 0,2 wie im Modell: σ_k = σ_1·(1 + h·((f_k/f_1)² − 1))")
    {
        const double t60 = 8.0;
        const double high = 0.2;
        const double h = (1.0 / high - 1.0) / 99.0;
        std::vector<Komponente> k = saite(110.0, 0.0, 30, t60);
        for (std::size_t i = 0; i < k.size(); ++i)
        {
            const double r = static_cast<double>(i + 1);
            k[i].t60Frueh = k[i].t60Spaet = t60 / (1.0 + h * (r * r - 1.0));
        }
        const Ergebnis e = miss(erzeuge(k, 8.0, 0.1, 1e-7), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        REQUIRE(e.hochErkannt);
        MESSAGE("highDecay gemessen " << e.highDecay << " (σ = " << e.sigmaB1 << " + " << e.sigmaB3 << "·ω²)");
        // eine Stufe je Teilton: auch wo der Teilton frueh im Boden ankommt (langsame tiefe
        // Teiltoene legen ihr Leck darunter), wird keine zweite erfunden
        for (const Teilton& t : e.teiltoene)
        {
            CAPTURE(t.k);
            CHECK_FALSE(t.abklingen.zweiStufen);
        }
        CHECK(e.highDecay == doctest::Approx(high).epsilon(0.05).scale(0.0));
        for (int n = 1; n <= 10; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK(t->abklingen.t60Frueh == doctest::Approx(k[static_cast<std::size_t>(n - 1)].t60Frueh).epsilon(0.03).scale(0.0));
        }
    }
}

TEST_CASE("Loop Teiltonmessung 31d-2 das Rauschen begrenzt den Bereich")
{
    // Teilton 1 bei −10,5 dB faellt mit 60 dB in 2 s; Rauschen ±1e-4: der Bereich endet, bevor die
    // Huellkurve im Rauschen liegt, und nach mindestens 40 dB
    const Ergebnis e = miss(erzeuge(saite(110.0, 0.0, 6, 2.0), 5.0, 0.1, 1e-4), kRate, 110.0, lumi::loop::fft());
    REQUIRE(e.erkannt);
    const Teilton* t = e.teilton(1);
    REQUIRE(t != nullptr);
    REQUIRE(t->abklingen.erkannt);
    MESSAGE("Rauschen " << t->rauschenDb << " dB, Bereich " << t->abklingen.vonS << " bis " << t->abklingen.bisS << " s");
    CHECK(t->rauschenDb < t->anfangDb - 50.0);
    CHECK(t->abklingen.bisS < 4.9);
    CHECK(t->abklingen.bisS > 40.0 / 30.0);  // 30 dB je Sekunde
    const auto ende = static_cast<std::size_t>(std::lround(t->abklingen.bisS / e.schrittS));
    CHECK(t->pegelDb[ende] > t->rauschenDb + 10.0);
    CHECK(t->abklingen.t60Frueh == doctest::Approx(2.0).epsilon(0.03).scale(0.0));
}

TEST_CASE("Loop Teiltonmessung 31d-3 Blende am Ende")
{
    // 12 s, T60 10 s; ab 9 s linear auf null ausgeblendet wie die Dateien von Salamander
    std::vector<double> ton = erzeuge(saite(220.0, 0.0, 8, 10.0), 12.0, 0.1, 1e-6);
    const std::size_t ohne = endeVorBlende(ton, kRate);
    CHECK(ohne == ton.size());
    for (std::size_t i = static_cast<std::size_t>(9.0 * kRate); i < ton.size(); ++i)
        ton[i] *= 1.0 - (static_cast<double>(i) / kRate - 9.0) / 3.0;
    const std::size_t ende = endeVorBlende(ton, kRate);
    MESSAGE("Blende ab 9 s: gemessen bis " << static_cast<double>(ende) / kRate << " s");
    CHECK(static_cast<double>(ende) / kRate > 7.5);
    CHECK(static_cast<double>(ende) / kRate < 9.0);
    const Ergebnis e = miss(std::span<const double>(ton).first(ende), kRate, 220.0, lumi::loop::fft());
    REQUIRE(e.erkannt);
    for (int n = 1; n <= 6; ++n)
    {
        CAPTURE(n);
        const Teilton* t = e.teilton(n);
        REQUIRE(t != nullptr);
        REQUIRE(t->abklingen.erkannt);
        CHECK_FALSE(t->abklingen.zweiStufen);
        CHECK(t->abklingen.t60Frueh == doctest::Approx(10.0).epsilon(0.03).scale(0.0));
    }
}

TEST_CASE("Loop Teiltonmessung 31d-3 Pegel, Direktschall, Terzbaender, Phantome")
{
    const lumi::loop::Fft& fft = lumi::loop::fft();
    SUBCASE("Pegel zu Beginn und Terzband eines Sinus")
    {
        const std::vector<double> ton = erzeuge({{1000.0, 0.5, 100.0, 100.0}}, 2.0, 0.1, 0.0);
        const Ergebnis e = miss(ton, kRate, 1000.0, fft);
        REQUIRE(e.erkannt);
        CHECK(std::abs(pegelZuBeginn(ton, kRate, e.einsatzS) - 20.0 * std::log10(0.5)) < 0.1);
        const std::vector<double> terz = terzPegel(ton, kRate, e.einsatzS, fft);
        const std::vector<double> mitten = terzMitten();
        REQUIRE(terz.size() == mitten.size());
        CHECK(mitten.front() == doctest::Approx(31.25).epsilon(0.01));
        CHECK(mitten.back() == doctest::Approx(10079.4).epsilon(0.01));
        const auto groesstes = static_cast<std::size_t>(std::max_element(terz.begin(), terz.end()) - terz.begin());
        CHECK(mitten[groesstes] == doctest::Approx(1000.0));
        CHECK(terz[groesstes - 1] < terz[groesstes] - 30.0);
        CHECK(terz[groesstes + 1] < terz[groesstes] - 30.0);
    }
    SUBCASE("Direktschall: ein Schlag vor dem Ton hebt ihn, der Ton allein nicht")
    {
        std::vector<double> ton = erzeuge(saite(220.0, 0.0, 10, 10.0), 1.0, 0.1, 0.0);
        const Ergebnis e = miss(ton, kRate, 220.0, fft);
        REQUIRE(e.erkannt);
        const double ohne = direktschallDb(ton, kRate, e.einsatzS);
        // 3 ms Rauschen mit dem Zehnfachen der Spitze ab dem Einsatz
        std::uint64_t z = 7;
        for (std::size_t i = 0; i < static_cast<std::size_t>(0.003 * kRate); ++i)
        {
            z = z * 6364136223846793005ull + 1442695040888963407ull;
            ton[static_cast<std::size_t>(0.1 * kRate) + i] += 3.0 * (static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0);
        }
        const Ergebnis mitSchlag = miss(ton, kRate, 220.0, fft);
        const double mit = direktschallDb(ton, kRate, mitSchlag.einsatzS);
        MESSAGE("Direktschall ohne " << ohne << " dB, mit Schlag " << mit << " dB");
        CHECK(std::abs(ohne) < 3.0);
        CHECK(mit > ohne + 10.0);
    }
    SUBCASE("Phantome bei 2·f(n/2), 20 dB unter dem Querteilton n")
    {
        const double f0 = 55.0;
        const double b = 4e-4;
        std::vector<Komponente> k = saite(f0, b, 20, 20.0);
        for (int n = 4; n <= 20; n += 2)
            k.push_back({2.0 * teiltonFrequenz(f0, b, n / 2), 0.1 * 0.3 / n, 20.0, 20.0});
        const std::vector<double> ton = erzeuge(k, 5.0, 0.1, 1e-6);
        const Ergebnis e = miss(ton, kRate, f0 * std::sqrt(1.0 + b), fft);
        REQUIRE(e.erkannt);
        const std::vector<Phantom> p = phantome(ton, kRate, e, fft);
        // n = 4 liegt 0,5 Hz neben dem Querteilton — naeher als die Aufloesung von 1 Hz: nicht gezaehlt
        REQUIRE(p.size() == 8);
        CHECK(p.front().n == 6);
        for (const Phantom& ph : p)
        {
            CAPTURE(ph.n);
            CHECK(ph.frequenz == doctest::Approx(2.0 * teiltonFrequenz(f0, b, ph.n / 2)).epsilon(1e-4).scale(0.0));
            CHECK(std::abs(ph.abstandDb + 20.0) < 1.0);
        }
    }
}

namespace
{

/// Die Kurve nach Schroeder einer Summe von Exponentialen in der Energie (ohne Mischterme): L(t) = 10·lg(I(t)/I(t0)),
/// I(t) = Σ a²/(2σ)·(e^(−2σt) − e^(−2σ·T)) mit σ = ln 1000 / T60.
double schroederWahr(const std::vector<std::pair<double, double>>& amplitudeT60, double t, double t0, double ende)
{
    const auto integral = [&](double von) {
        double s = 0.0;
        for (const auto& [a, t60] : amplitudeT60)
        {
            const double sigma = std::log(1000.0) / t60;
            s += a * a / (2.0 * sigma) * (std::exp(-2.0 * sigma * von) - std::exp(-2.0 * sigma * ende));
        }
        return s;
    };
    return 10.0 * std::log10(integral(t) / integral(t0));
}

/// Die groesste Abweichung der Kurve von `wahr` ueber ihre gueltigen Punkte.
double groessteAbweichung(const Abklingkurve& k, const std::function<double(double, double)>& wahr, double endeS)
{
    double groesste = 0.0;
    for (std::size_t j = 0; j < k.gueltig; ++j)
    {
        const double t = static_cast<double>(k.ersterPunkt + j) * k.rasterS;
        groesste = std::max(groesste, std::abs(k.db[j] - wahr(t, endeS)));
    }
    return groesste;
}

}  // namespace

TEST_CASE("Loop Teiltonmessung 31d-7 B Kurve nach Schroeder (7.5.1)")
{
    // SM4 a, K5 a, K6 a (S155): die Kurve auf dem Raster 0,05 s, bis zu einem waehlbaren Ende, ohne die Energie des Bodens
    const double schritt = Einstellungen{}.schrittS;
    SUBCASE("eine Stufe: 4 s — eine Gerade mit −15 dB/s, Bereich wie die Abklingzeit")
    {
        const Ergebnis e = miss(erzeuge(saite(110.0, 1e-4, 8, 4.0), 6.0, 0.1, 1e-6), kRate, 110.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 3; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            REQUIRE(t->abklingen.erkannt);
            CHECK(t->bodenDb.size() == t->pegelDb.size());
            CHECK(static_cast<double>(t->bereichVon) * schritt == doctest::Approx(t->abklingen.vonS).epsilon(1e-12));
            CHECK(static_cast<double>(t->bereichBis - 1) * schritt == doctest::Approx(t->abklingen.bisS).epsilon(1e-12));
            const Abklingkurve k = schroederKurve(*t, schritt, t->bereichVon, t->bereichBis);
            REQUIRE(k.gueltig >= 20);
            CHECK(k.db.front() == 0.0);
            CHECK(static_cast<double>(k.ersterPunkt) * k.rasterS >= static_cast<double>(t->bereichVon) * schritt - 1e-12);
            const double endeS = static_cast<double>(t->bereichBis) * schritt;
            const double abweichung = groessteAbweichung(
                k, [&](double tt, double ende) { return schroederWahr({{1.0, 4.0}}, tt, static_cast<double>(k.ersterPunkt) * k.rasterS, ende); },
                endeS);
            MESSAGE("Teilton " << n << ": " << k.db.size() << " Punkte, gueltig " << k.gueltig << ", groesste Abweichung " << abweichung << " dB");
            CHECK(abweichung < 0.5);
        }
    }
    SUBCASE("zwei Stufen: frueh 3 s, spaet 12 s, die spaete 30 dB leiser")
    {
        const Ergebnis e = miss(erzeuge(zweiModen(110.0, 1e-4, 12, 3.0, 12.0, -30.0, 2.0), 8.0, 0.1, 1e-6), kRate, 110.0,
                                lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 3; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            const Abklingkurve k = schroederKurve(*t, schritt, t->bereichVon, t->bereichBis);
            REQUIRE(k.gueltig >= 20);
            const double endeS = static_cast<double>(t->bereichBis) * schritt;
            const double a2 = std::pow(10.0, -30.0 / 20.0);
            const double abweichung = groessteAbweichung(
                k,
                [&](double tt, double ende) {
                    return schroederWahr({{1.0, 3.0}, {a2, 12.0}}, tt, static_cast<double>(k.ersterPunkt) * k.rasterS, ende);
                },
                endeS);
            MESSAGE("Teilton " << n << ": gueltig " << k.gueltig << " von " << k.db.size() << ", groesste Abweichung " << abweichung << " dB");
            CHECK(abweichung < 1.0);
        }
    }
    SUBCASE("der Boden: bis in das Rauschen integriert, biegt es die Kurve ohne Abzug flach, mit Abzug weniger")
    {
        // ein gemeinsames Ende (K6 a) kann hinter dem eigenen Bereich im Rauschen liegen — dort traegt der Abzug
        const std::vector<double> ton = erzeuge(saite(220.0, 0.0, 6, 3.0), 6.0, 0.1, 3e-4);
        const Ergebnis e = miss(ton, kRate, 220.0, lumi::loop::fft());
        REQUIRE(e.erkannt);
        for (int n = 1; n <= 2; ++n)
        {
            CAPTURE(n);
            const Teilton* t = e.teilton(n);
            REQUIRE(t != nullptr);
            const std::size_t ende = t->pegelDb.size();
            const Abklingkurve mit = schroederKurve(*t, schritt, t->bereichVon, ende);
            const Abklingkurve ohne = schroederKurve(*t, schritt, t->bereichVon, ende, 0.05, false);
            REQUIRE(mit.gueltig >= 20);
            const double endeS = static_cast<double>(ende) * schritt;
            const auto wahr = [&](double tt, double ende) {
                return schroederWahr({{1.0, 3.0}}, tt, static_cast<double>(mit.ersterPunkt) * mit.rasterS, ende);
            };
            const double abwMit = groessteAbweichung(mit, wahr, endeS);
            const double abwOhne = groessteAbweichung(ohne, wahr, endeS);
            MESSAGE("Teilton " << n << ": bis " << endeS << " s, mit Abzug " << abwMit << " dB, ohne " << abwOhne << " dB");
            // gemessen S155 mit K6-1 a (bis 1 s vor dem Ende, tief im Rauschen): 11,2 gegen 17,6 dB, 17,3 gegen 23,6 dB
            CHECK(abwMit < abwOhne);

            // bis zum eigenen Bereich (10 dB ueber dem Boden), wie bei Salamander
            const Abklingkurve mitB = schroederKurve(*t, schritt, t->bereichVon, t->bereichBis);
            const Abklingkurve ohneB = schroederKurve(*t, schritt, t->bereichVon, t->bereichBis, 0.05, false);
            const double bisB = static_cast<double>(t->bereichBis) * schritt;
            const auto wahrB = [&](double tt, double ende) {
                return schroederWahr({{1.0, 3.0}}, tt, static_cast<double>(mitB.ersterPunkt) * mitB.rasterS, ende);
            };
            MESSAGE("Teilton " << n << ": im eigenen Bereich bis " << bisB << " s, mit Abzug " << groessteAbweichung(mitB, wahrB, bisB)
                               << " dB, ohne " << groessteAbweichung(ohneB, wahrB, bisB) << " dB");
        }
    }
    SUBCASE("ein gemeinsames Ende hinter dem eigenen Bereich verlaengert die Kurve")
    {
        const Ergebnis e = miss(erzeuge(saite(110.0, 1e-4, 8, 4.0), 6.0, 0.1, 1e-6), kRate, 110.0, lumi::loop::fft());
        const Teilton* t = e.teilton(1);
        REQUIRE(t != nullptr);
        const Abklingkurve eigen = schroederKurve(*t, schritt, t->bereichVon, t->bereichBis);
        const Abklingkurve weiter = schroederKurve(*t, schritt, t->bereichVon, t->pegelDb.size());
        CHECK(weiter.db.size() >= eigen.db.size());
        CHECK(weiter.ersterPunkt == eigen.ersterPunkt);
    }
}

TEST_CASE("Loop Teiltonmessung 31d-7 B Autokovarianz des Rests (7.5.1)")
{
    // K3 a (S155): zwei Moden 1 Hz auseinander, die zweite halb so laut — Einbrueche 20·lg 3 = 9,5 dB tief im Takt 1 s;
    // ohne zweite Mode kein Rest
    const double schritt = Einstellungen{}.schrittS;
    std::vector<Komponente> k = saite(220.0, 0.0, 6, 20.0);
    for (const Komponente& c : saite(220.0, 0.0, 6, 20.0)) k.push_back({c.frequenz + 1.0, 0.5 * c.amplitude, 20.0, 20.0});
    const Ergebnis e = miss(erzeuge(k, 8.0, 0.1, 1e-6), kRate, 220.0, lumi::loop::fft());
    const Ergebnis allein = miss(erzeuge(saite(220.0, 0.0, 6, 20.0), 8.0, 0.1, 1e-6), kRate, 220.0, lumi::loop::fft());
    REQUIRE(e.erkannt);
    REQUIRE(allein.erkannt);
    for (int n = 1; n <= 2; ++n)
    {
        CAPTURE(n);
        const Teilton* t = e.teilton(n);
        const Teilton* a = allein.teilton(n);
        REQUIRE(t != nullptr);
        REQUIRE(a != nullptr);
        const Autokovarianz c = autokovarianz(*t, schritt, t->bereichVon, t->bereichBis);
        const Autokovarianz c0 = autokovarianz(*a, schritt, a->bereichVon, a->bereichBis);
        REQUIRE(c.db2.size() == 101);  // 0 … 2 s im Raster 0,02 s
        const double periode = 1.0;  // jeder Teilton hat seine zweite Mode 1 Hz daneben
        const auto bei = [&](double tau) { return c.db2[static_cast<std::size_t>(std::lround(tau / c.rasterS))]; };
        MESSAGE("Teilton " << n << ": C(0) " << c.db2[0] << " dB², C(T/2) " << bei(0.5 * periode) << ", C(T) " << bei(periode)
                           << "; allein C(0) " << c0.db2[0]);
        CHECK(bei(0.5 * periode) < -0.5 * c.db2[0]);
        CHECK(bei(periode) > 0.5 * c.db2[0]);
        CHECK(c0.db2[0] < 0.01 * c.db2[0]);
    }
}
