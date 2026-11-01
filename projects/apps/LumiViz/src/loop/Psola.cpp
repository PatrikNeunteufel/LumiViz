/**
 * @file Psola.cpp
 * @brief Loop: TD-PSOLA fuer einstimmigen Gesang (Stufe 15h, S170)
 */

#include "loop/Psola.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace lumi::loop
{

namespace
{

/// Bis hierhin glaettet der Tiefpass vor der Suche nach dem Gipfel (von Claude gesetzt): ueber
/// dem Grundton jeder Stimme, unter den meisten zweiten Formanten — der Gipfel je Periode ist
/// dann der Puls, nicht ein Nachschwingen des Formanten.
constexpr double kGlaettungHz = 1000.0;
/// So weit (Anteil der Periode) um die erwartete Stelle wird der Gipfel gesucht.
constexpr double kSuchAnteil = 0.2;

/// Das Lesen auf Bruchteile: gefensterter Sinc, 16 Stuetzen (k = −7 … 8 um den Abtastwert vor der
/// Stelle), Kaiser β = 8, 1024 Phasen, je Phase auf die Summe 1 gebracht (von Claude gesetzt).
constexpr int kStuetzen = 16;
constexpr int kPhasen = 1024;
constexpr double kKaiserBeta = 8.0;

/// I0, die modifizierte Besselfunktion erster Art, als Reihe (fuer das Kaiser-Fenster).
double besselI0(double x)
{
    double summe = 1.0;
    double glied = 1.0;
    for (int k = 1; k < 50; ++k)
    {
        glied *= (x / (2.0 * k)) * (x / (2.0 * k));
        summe += glied;
        if (glied < 1e-17 * summe) break;
    }
    return summe;
}

using SincTafel = std::array<std::array<double, kStuetzen>, kPhasen>;

const SincTafel& sincTafel()
{
    static const SincTafel tafel = [] {
        SincTafel t{};
        const double halb = kStuetzen / 2.0;
        const double norm = besselI0(kKaiserBeta);
        for (int p = 0; p < kPhasen; ++p)
        {
            const double bruch = static_cast<double>(p) / kPhasen;
            double summe = 0.0;
            for (int k = 0; k < kStuetzen; ++k)
            {
                const double x = bruch - static_cast<double>(k - (kStuetzen / 2 - 1));  // Abstand zur Stuetze k − 7
                const double sinc = x == 0.0 ? 1.0 : std::sin(std::numbers::pi * x) / (std::numbers::pi * x);
                const double r = x / halb;
                const double fenster = std::abs(r) >= 1.0 ? 0.0 : besselI0(kKaiserBeta * std::sqrt(1.0 - r * r)) / norm;
                t[static_cast<std::size_t>(p)][static_cast<std::size_t>(k)] = sinc * fenster;
                summe += sinc * fenster;
            }
            for (double& w : t[static_cast<std::size_t>(p)]) w /= summe;
        }
        return t;
    }();
    return tafel;
}

/// Ein Kanal (`kanal` 0 oder 1) von `stereo` an der Stelle `q` mit Bruchteil; ausserhalb Stille.
double lies(const std::vector<float>& stereo, std::int64_t n, int kanal, double q)
{
    auto unten = static_cast<std::int64_t>(std::floor(q));
    auto phase = std::llround((q - static_cast<double>(unten)) * kPhasen);
    if (phase == kPhasen)
    {
        phase = 0;
        ++unten;
    }
    const auto& w = sincTafel()[static_cast<std::size_t>(phase)];
    double summe = 0.0;
    for (int k = 0; k < kStuetzen; ++k)
    {
        const std::int64_t i = unten + k - (kStuetzen / 2 - 1);
        if (i < 0 || i >= n) continue;
        summe += w[static_cast<std::size_t>(k)] * stereo[static_cast<std::size_t>(2 * i + kanal)];
    }
    return summe;
}

/// Ein Pol vorwaerts und rueckwaerts: ohne Versatz in der Zeit.
std::vector<double> geglaettet(const std::vector<double>& x, int rate)
{
    const double a = std::exp(-2.0 * std::numbers::pi * kGlaettungHz / rate);
    std::vector<double> y(x.size());
    double z = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) y[i] = z = (1.0 - a) * x[i] + a * z;
    z = 0.0;
    for (std::size_t i = x.size(); i-- > 0;) y[i] = z = (1.0 - a) * y[i] + a * z;
    return y;
}

/// Die Frequenz des Verlaufs an Abtastwert `t`; 0 = nicht gesungen. Zwischen zwei gesungenen
/// Rahmen linear, sonst der naechste Rahmen.
double hzBei(const Tonhoehenverlauf& v, double t)
{
    if (v.hz.empty() || v.schritt <= 0) return 0.0;
    const double j = t / v.schritt;
    const auto letzter = static_cast<std::int64_t>(v.hz.size()) - 1;
    const auto unten = std::clamp<std::int64_t>(static_cast<std::int64_t>(std::floor(j)), 0, letzter);
    const auto oben = std::min(unten + 1, letzter);
    const double a = v.hz[static_cast<std::size_t>(unten)];
    const double b = v.hz[static_cast<std::size_t>(oben)];
    if (a > 0.0 && b > 0.0)
    {
        const double f = std::clamp(j - static_cast<double>(unten), 0.0, 1.0);
        return a + (b - a) * f;
    }
    return j - static_cast<double>(unten) < 0.5 ? a : b;
}

/// Wo im Bereich [von, bis] das Signal am hoechsten ist, parabolisch ueber die Nachbarn verfeinert.
double gipfel(const std::vector<double>& x, std::int64_t von, std::int64_t bis)
{
    std::int64_t beste = von;
    for (std::int64_t i = von + 1; i <= bis; ++i)
        if (x[static_cast<std::size_t>(i)] > x[static_cast<std::size_t>(beste)]) beste = i;
    if (beste <= 0 || beste + 1 >= static_cast<std::int64_t>(x.size())) return static_cast<double>(beste);
    const double l = x[static_cast<std::size_t>(beste - 1)];
    const double m = x[static_cast<std::size_t>(beste)];
    const double r = x[static_cast<std::size_t>(beste + 1)];
    const double nenner = l - 2.0 * m + r;
    return static_cast<double>(beste) + (nenner < 0.0 ? std::clamp(0.5 * (l - r) / nenner, -0.5, 0.5) : 0.0);
}

/// Hann je Haelfte: 1 an der Marke, 0 an der Nachbarmarke.
double fenster(double anteil)
{
    return 0.5 * (1.0 + std::cos(std::numbers::pi * anteil));
}

class DehnerPsola final : public Dehner
{
public:
    explicit DehnerPsola(const PYinEinstellungen& pyin) : m_pyin(pyin) {}

    [[nodiscard]] std::string name() const override { return m_pyin.nurImBereich ? "TD-PSOLA, pYIN in range" : "TD-PSOLA"; }
    [[nodiscard]] bool behaeltFormanten() const override { return true; }

    [[nodiscard]] std::vector<float> rechne(const std::vector<float>& stereo, int rate, std::int64_t ausFrames,
                                            const std::function<double(std::int64_t)>& halbtoene) const override
    {
        if (ausFrames <= 0 || rate <= 0) return {};
        std::vector<float> ergebnis(static_cast<std::size_t>(2 * ausFrames), 0.0f);
        const auto n = static_cast<std::int64_t>(stereo.size() / 2);
        if (n == 0) return ergebnis;

        // die Tonhoehe je Block, gefragt in der Mitte des Blocks (Vertrag von `Dehner`)
        std::vector<double> beta(static_cast<std::size_t>((ausFrames + kDehnerBlock - 1) / kDehnerBlock), 1.0);
        for (std::size_t b = 0; b < beta.size(); ++b)
        {
            const auto k = static_cast<std::int64_t>(b) * kDehnerBlock;
            const std::int64_t block = std::min<std::int64_t>(kDehnerBlock, ausFrames - k);
            // begrenzt auf drei Oktaven, damit der Abstand der Koerner nie gegen null geht
            beta[b] = halbtoene ? std::pow(2.0, std::clamp(halbtoene(k + block / 2), -36.0, 36.0) / 12.0) : 1.0;
        }

        std::vector<double> mono(static_cast<std::size_t>(n));
        for (std::int64_t i = 0; i < n; ++i)
            mono[static_cast<std::size_t>(i)] =
                0.5 * (static_cast<double>(stereo[static_cast<std::size_t>(2 * i)]) + stereo[static_cast<std::size_t>(2 * i + 1)]);
        const std::vector<PsolaMarke> marken = psolaMarken(mono, rate, messeTonhoehenverlauf(stereo, rate, m_pyin));
        const auto m = static_cast<std::int64_t>(marken.size());
        const auto stelle = [&marken](std::int64_t i) { return marken[static_cast<std::size_t>(i)].stelle; };

        std::vector<double> links(static_cast<std::size_t>(ausFrames), 0.0);
        std::vector<double> rechts(static_cast<std::size_t>(ausFrames), 0.0);
        std::vector<double> summe(static_cast<std::size_t>(ausFrames), 0.0);
        const double verhaeltnis = static_cast<double>(n) / static_cast<double>(ausFrames);
        const double stumm = std::max(1.0, std::round(kPsolaStummAbstandS * rate));

        double ts = stelle(0) / verhaeltnis;
        while (ts < static_cast<double>(ausFrames))
        {
            // die Marke, die der Stelle der Eingabe am naechsten liegt
            const double ta = ts * verhaeltnis;
            auto i = static_cast<std::int64_t>(
                std::lower_bound(marken.begin(), marken.end(), ta, [](const PsolaMarke& mk, double t) { return mk.stelle < t; }) -
                marken.begin());
            if (i == m || (i > 0 && ta - stelle(i - 1) < stelle(i) - ta)) --i;
            const PsolaMarke& marke = marken[static_cast<std::size_t>(i)];
            // mindestens ein Abtastwert, damit der Schritt nie null wird
            const double nachRechts =
                std::max(1.0, i + 1 < m ? stelle(i + 1) - marke.stelle : (i > 0 ? marke.stelle - stelle(i - 1) : stumm));
            const double nachLinks = std::max(1.0, i > 0 ? marke.stelle - stelle(i - 1) : nachRechts);

            const auto block = std::min<std::size_t>(static_cast<std::size_t>(ts) / kDehnerBlock, beta.size() - 1);
            const double b = marke.gesungen ? beta[block] : 1.0;
            const double faktor = b < 1.0 ? std::sqrt(1.0 / b) : 1.0;
            // das Korn liegt auf ts (mit Bruchteil); Abtastwert o liest den Klang an marke + (o − ts)
            const auto von = std::max<std::int64_t>(0, static_cast<std::int64_t>(std::floor(ts - nachLinks)) + 1);
            const auto bis = std::min<std::int64_t>(ausFrames - 1, static_cast<std::int64_t>(std::ceil(ts + nachRechts)) - 1);
            for (std::int64_t o = von; o <= bis; ++o)
            {
                const double j = static_cast<double>(o) - ts;
                if (j <= -nachLinks || j >= nachRechts) continue;
                const double w = j < 0.0 ? fenster(-j / nachLinks) : fenster(j / nachRechts);
                const double q = marke.stelle + j;
                links[static_cast<std::size_t>(o)] += faktor * w * lies(stereo, n, 0, q);
                rechts[static_cast<std::size_t>(o)] += faktor * w * lies(stereo, n, 1, q);
                summe[static_cast<std::size_t>(o)] += w;
            }
            ts += nachRechts / b;
        }

        for (std::int64_t k = 0; k < ausFrames; ++k)
        {
            const double teiler = std::max(1.0, summe[static_cast<std::size_t>(k)]);
            ergebnis[static_cast<std::size_t>(2 * k)] = static_cast<float>(links[static_cast<std::size_t>(k)] / teiler);
            ergebnis[static_cast<std::size_t>(2 * k + 1)] = static_cast<float>(rechts[static_cast<std::size_t>(k)] / teiler);
        }
        return ergebnis;
    }

private:
    PYinEinstellungen m_pyin;
};

}  // namespace

std::vector<PsolaMarke> psolaMarken(const std::vector<double>& mono, int rate, const Tonhoehenverlauf& verlauf)
{
    std::vector<PsolaMarke> marken;
    const auto n = static_cast<std::int64_t>(mono.size());
    if (n == 0 || rate <= 0) return marken;
    const std::vector<double> glatt = geglaettet(mono, rate);
    const auto stumm = std::max<std::int64_t>(1, std::llround(kPsolaStummAbstandS * rate));

    std::int64_t t = 0;  // ab hier wird die naechste Marke gesucht
    while (t < n)
    {
        const double hz = hzBei(verlauf, static_cast<double>(t));
        if (hz <= 0.0)
        {
            // nach einer gesungenen Marke nicht gleich daneben, sondern im Abstand der stummen
            const std::int64_t s =
                marken.empty() ? t : std::max(t, static_cast<std::int64_t>(std::ceil(marken.back().stelle)) + stumm);
            if (s >= n) break;
            marken.push_back({static_cast<double>(s), false});
            t = s + stumm;
            continue;
        }
        const double periode = rate / hz;
        std::int64_t von = t;
        std::int64_t bis = t + std::llround(periode) - 1;
        if (!marken.empty() && marken.back().gesungen)
        {
            // um die erwartete Stelle: eine Periode nach der vorigen Marke
            const double erwartet = marken.back().stelle + periode;
            von = std::max(static_cast<std::int64_t>(std::floor(marken.back().stelle)) + 1,
                           std::llround(erwartet - kSuchAnteil * periode));
            bis = std::llround(erwartet + kSuchAnteil * periode);
        }
        bis = std::min(bis, n - 1);
        if (von > bis) break;
        const double g = gipfel(glatt, von, bis);
        marken.push_back({g, true});
        t = static_cast<std::int64_t>(std::floor(g)) + 1;
    }
    if (marken.empty()) marken.push_back({0.0, false});
    return marken;
}

std::unique_ptr<Dehner> dehnerPsola(const PYinEinstellungen& pyin)
{
    return std::make_unique<DehnerPsola>(pyin);
}

}  // namespace lumi::loop
