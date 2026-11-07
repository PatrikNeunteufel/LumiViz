/**
 * @file PYin.cpp
 * @brief Loop: der Tonhoehenverlauf mit pYIN nach Mauch & Dixon 2014 (Stufe 15g, S169)
 */

#include "loop/PYin.hpp"

#include "InstrumentCore/Fft.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <map>

namespace lumi::loop
{

namespace
{

constexpr int kSchwellen = 100;  ///< 0,01 … 1 in Schritten von 0,01 (§2.1)
constexpr double kMinusUnendlich = -std::numeric_limits<double>::infinity();

/// Rahmen (halbe Laenge W) und Schritt bei `rate`: dieselben Zeiten wie 2048 / 256 bei 44,1 kHz.
struct Masse
{
    std::int64_t halb = 0;
    std::int64_t schritt = 0;
};

Masse masseBei(int rate)
{
    return {std::max<std::int64_t>(64, std::llround(kPYinRahmen441 / 2.0 * rate / 44100.0)),
            std::max<std::int64_t>(1, std::llround(static_cast<double>(kPYinSchritt441) * rate / 44100.0))};
}

/// Die Gewichte der Schwellen: Beta(2, β) mit dem verlangten Mittelwert, an 0,01 … 1 genommen
/// und auf 1 gebracht (§2.1, Bild 2; α = 2, siehe Kopf von PYin.hpp).
std::array<double, kSchwellen> schwellenGewichte(double mittel)
{
    const double beta = 2.0 / std::clamp(mittel, 0.01, 0.9) - 2.0;
    std::array<double, kSchwellen> p{};
    double summe = 0.0;
    for (int i = 0; i < kSchwellen; ++i)
    {
        const double s = (i + 1) / 100.0;
        p[static_cast<std::size_t>(i)] = s * std::pow(1.0 - s, beta - 1.0);
        summe += p[static_cast<std::size_t>(i)];
    }
    for (double& w : p) w /= summe;
    return p;
}

/// Die normierte Differenzfunktion d′(τ), τ = 0 … W − 1, fuer den Rahmen ab `anfang` (2W Werte,
/// ausserhalb von `mono` Stille). d(τ) = r(0) + r_τ(0) − 2 r(τ) (Gl. 3), r ueber die FFT.
std::vector<double> normierteDifferenz(const std::vector<double>& mono, std::int64_t anfang, std::int64_t halb)
{
    const std::size_t laenge = kern::zweierpotenzAb(static_cast<std::size_t>(2 * halb));
    std::vector<double> a(laenge, 0.0);
    std::vector<double> b(laenge, 0.0);
    const auto n = static_cast<std::int64_t>(mono.size());
    for (std::int64_t j = 0; j < 2 * halb; ++j)
    {
        const std::int64_t i = anfang + j;
        const double x = i >= 0 && i < n ? mono[static_cast<std::size_t>(i)] : 0.0;
        b[static_cast<std::size_t>(j)] = x;
        if (j < halb) a[static_cast<std::size_t>(j)] = x;
    }
    std::vector<std::complex<double>> fa(laenge / 2 + 1);
    std::vector<std::complex<double>> fb(laenge / 2 + 1);
    kern::fft().vorwaerts(a, fa);
    kern::fft().vorwaerts(b, fb);
    for (std::size_t k = 0; k < fa.size(); ++k) fa[k] = std::conj(fa[k]) * fb[k];
    std::vector<double> r(laenge, 0.0);
    kern::fft().rueckwaerts(fa, r);  // r(τ) = Σ_j x_j x_{j+τ}, j < W (Gl. 2)

    // die Energie je Fenster der Laenge W ab τ: r_τ(0)
    std::vector<double> quadratSumme(static_cast<std::size_t>(2 * halb + 1), 0.0);
    for (std::int64_t j = 0; j < 2 * halb; ++j)
        quadratSumme[static_cast<std::size_t>(j + 1)] = quadratSumme[static_cast<std::size_t>(j)] + b[static_cast<std::size_t>(j)] * b[static_cast<std::size_t>(j)];
    const auto energie = [&](std::int64_t tau) {
        return quadratSumme[static_cast<std::size_t>(tau + halb)] - quadratSumme[static_cast<std::size_t>(tau)];
    };

    std::vector<double> dn(static_cast<std::size_t>(halb), 1.0);
    double summe = 0.0;
    for (std::int64_t tau = 1; tau < halb; ++tau)
    {
        const double d = std::max(0.0, energie(0) + energie(tau) - 2.0 * r[static_cast<std::size_t>(tau)]);
        summe += d;
        dn[static_cast<std::size_t>(tau)] = summe > 0.0 ? d * static_cast<double>(tau) / summe : 1.0;
    }
    return dn;
}

/// Die Frequenz der Periode τ, parabolisch zwischen den Nachbarn verfeinert wie bei YIN.
double verfeinertHz(const std::vector<double>& dn, int rate, std::int64_t tau)
{
    const double l = dn[static_cast<std::size_t>(tau - 1)];
    const double m = dn[static_cast<std::size_t>(tau)];
    const double r = dn[static_cast<std::size_t>(tau + 1)];
    const double nenner = l - 2.0 * m + r;
    const double versatz = nenner > 0.0 ? std::clamp(0.5 * (l - r) / nenner, -0.5, 0.5) : 0.0;
    return rate / (static_cast<double>(tau) + versatz);
}

/// Stufe 1 an einer normierten Differenzfunktion (Gl. 4, 5) mit der **Talregel** (PY2 a, S170;
/// wie librosa `pyin`): je Schwelle s das erste Tal von d′ unter s mit einer Periode im Bereich
/// des HMM, mit seinem Gewicht; ohne eines das tiefste Tal im Bereich mit p_a. Die Frequenz
/// parabolisch verfeinert.
std::vector<PYinKandidat> kandidatenAus(const std::vector<double>& dn, int rate, const std::array<double, kSchwellen>& gewichte,
                                        double pa)
{
    const auto halb = static_cast<std::int64_t>(dn.size());
    if (halb < 4) return {};
    const auto von = std::max<std::int64_t>(2, static_cast<std::int64_t>(std::ceil(rate / pyinStufeHz(kPYinStufen - 1))));
    const auto bis = std::min<std::int64_t>(halb - 1, static_cast<std::int64_t>(std::floor(rate / kPYinUntenHz)) + 1);
    std::vector<std::int64_t> taeler;
    for (std::int64_t tau = von; tau < bis; ++tau)
        if (dn[static_cast<std::size_t>(tau)] <= dn[static_cast<std::size_t>(tau - 1)] &&
            dn[static_cast<std::size_t>(tau)] <= dn[static_cast<std::size_t>(tau + 1)])
            taeler.push_back(tau);
    std::vector<PYinKandidat> kandidaten;
    if (taeler.empty()) return kandidaten;
    const std::int64_t tiefstes = *std::min_element(taeler.begin(), taeler.end(), [&dn](std::int64_t a, std::int64_t b) {
        return dn[static_cast<std::size_t>(a)] < dn[static_cast<std::size_t>(b)];
    });
    std::map<std::int64_t, double> je;
    for (int i = 0; i < kSchwellen; ++i)
    {
        const double s = (i + 1) / 100.0;
        const auto erstes = std::find_if(taeler.begin(), taeler.end(), [&dn, s](std::int64_t t) { return dn[static_cast<std::size_t>(t)] < s; });
        if (erstes == taeler.end())
            je[tiefstes] += pa * gewichte[static_cast<std::size_t>(i)];
        else
            je[*erstes] += gewichte[static_cast<std::size_t>(i)];
    }
    for (const auto& [tau, p] : je) kandidaten.push_back({verfeinertHz(dn, rate, tau), p});
    return kandidaten;
}

std::vector<double> monoVon(const std::vector<float>& stereo)
{
    std::vector<double> mono(stereo.size() / 2);
    for (std::size_t i = 0; i < mono.size(); ++i) mono[i] = 0.5 * (static_cast<double>(stereo[2 * i]) + stereo[2 * i + 1]);
    return mono;
}

/// Die Stufe des HMM fuer eine Frequenz; −1 = ausserhalb (55 Hz … knapp 880 Hz).
int stufeVon(double hz)
{
    if (hz <= 0.0) return -1;
    const auto stufe = static_cast<int>(std::lround(120.0 * std::log2(hz / kPYinUntenHz)));
    return stufe >= 0 && stufe < kPYinStufen ? stufe : -1;
}

}  // namespace

double pyinStufeHz(int stufe)
{
    return kPYinUntenHz * std::pow(2.0, stufe / 120.0);
}

std::vector<PYinKandidat> pyinKandidaten(const std::vector<double>& mono, int rate, std::int64_t mitte,
                                         const PYinEinstellungen& einstellungen)
{
    if (rate <= 0) return {};
    const Masse m = masseBei(rate);
    return kandidatenAus(normierteDifferenz(mono, mitte - m.halb / 2, m.halb), rate, schwellenGewichte(einstellungen.schwelleMittel),
                         einstellungen.pa);
}

Tonhoehenverlauf messeTonhoehenverlauf(const std::vector<float>& stereo, int rate, const PYinEinstellungen& e)
{
    Tonhoehenverlauf verlauf;
    verlauf.rate = rate;
    if (rate <= 0 || stereo.size() < 2) return verlauf;
    const Masse m = masseBei(rate);
    verlauf.schritt = static_cast<int>(m.schritt);
    const std::vector<double> mono = monoVon(stereo);
    const auto rahmen = static_cast<std::int64_t>((static_cast<std::int64_t>(mono.size()) + m.schritt - 1) / m.schritt);
    const std::array<double, kSchwellen> gewichte = schwellenGewichte(e.schwelleMittel);

    // --- Stufe 1: je Rahmen die Kandidaten
    // d(τ) vergleicht x_j mit x_(j+τ) fuer j < W: gemessen wird die Periode um Anfang + (W + τ) / 2.
    // Der Rahmen beginnt deshalb W / 2 vor seiner Stelle — sie liegt dann τ / 2 hinter der
    // gemessenen (bei 220 Hz 2,3 ms; mit W vor der Stelle waeren es 11,6 ms zu frueh).
    std::vector<std::vector<PYinKandidat>> kandidaten(static_cast<std::size_t>(rahmen));
    for (std::int64_t k = 0; k < rahmen; ++k)
        kandidaten[static_cast<std::size_t>(k)] =
            kandidatenAus(normierteDifferenz(mono, k * m.schritt - m.halb / 2, m.halb), rate, gewichte, e.pa);

    // --- Stufe 2: das HMM (§2.2) — Zustand = Stufe + M · (gesungen ? 0 : 1)
    const int stufen = kPYinStufen;
    const int zustaende = 2 * stufen;
    const int sprung = std::max(0, e.sprungStufen);
    // die Dreiecksgewichte, je Ausgangsstufe ueber die erreichbaren Stufen auf 1 gebracht (Gl. 8)
    std::vector<double> logNorm(static_cast<std::size_t>(stufen));
    for (int i = 0; i < stufen; ++i)
    {
        double summe = 0.0;
        for (int d = -sprung; d <= sprung; ++d)
            if (i + d >= 0 && i + d < stufen) summe += sprung + 1 - std::abs(d);
        logNorm[static_cast<std::size_t>(i)] = std::log(summe);
    }
    std::vector<double> logDreieck(static_cast<std::size_t>(2 * sprung + 1));
    for (int d = -sprung; d <= sprung; ++d) logDreieck[static_cast<std::size_t>(d + sprung)] = std::log(static_cast<double>(sprung + 1 - std::abs(d)));
    const double logBleibt = std::log(1.0 - e.wechsel);
    const double logWechselt = std::log(e.wechsel);

    // die Beobachtung (Gl. 6): gesungen 0,5 · p*_m, nicht gesungen 0,5 · (1 − Σ p*)
    const auto beobachtung = [&](std::int64_t k, std::vector<double>& log) {
        log.assign(static_cast<std::size_t>(zustaende), kMinusUnendlich);
        double summe = 0.0;
        for (const PYinKandidat& c : kandidaten[static_cast<std::size_t>(k)])
            if (const int stufe = stufeVon(c.hz); stufe >= 0)
            {
                double& w = log[static_cast<std::size_t>(stufe)];
                w = std::isinf(w) ? 0.5 * c.p : w + 0.5 * c.p;  // zunaechst linear
                summe += c.p;
            }
        for (int i = 0; i < stufen; ++i)
            if (!std::isinf(log[static_cast<std::size_t>(i)])) log[static_cast<std::size_t>(i)] = std::log(log[static_cast<std::size_t>(i)]);
        const double ohne = std::log(std::max(1e-300, 0.5 * (1.0 - std::min(1.0, summe))));
        for (int i = stufen; i < zustaende; ++i) log[static_cast<std::size_t>(i)] = ohne;
        return summe;
    };

    std::vector<double> delta(static_cast<std::size_t>(zustaende));
    std::vector<double> neu(static_cast<std::size_t>(zustaende));
    std::vector<double> beob;
    std::vector<std::vector<std::int16_t>> zurueck(static_cast<std::size_t>(rahmen), std::vector<std::int16_t>(static_cast<std::size_t>(zustaende), 0));
    verlauf.stimmhaft.assign(static_cast<std::size_t>(rahmen), 0.0f);
    for (std::int64_t k = 0; k < rahmen; ++k)
    {
        verlauf.stimmhaft[static_cast<std::size_t>(k)] = static_cast<float>(std::min(1.0, beobachtung(k, beob)));
        if (k == 0)
        {
            // am Anfang gleich verteilt ueber die Zustaende nicht gesungen
            for (int z = 0; z < zustaende; ++z)
                delta[static_cast<std::size_t>(z)] = z >= stufen ? beob[static_cast<std::size_t>(z)] - std::log(static_cast<double>(stufen)) : kMinusUnendlich;
            continue;
        }
        std::vector<std::int16_t>& von = zurueck[static_cast<std::size_t>(k)];
        for (int z = 0; z < zustaende; ++z)
        {
            if (std::isinf(beob[static_cast<std::size_t>(z)]))
            {
                neu[static_cast<std::size_t>(z)] = kMinusUnendlich;
                continue;
            }
            const int j = z % stufen;
            const bool zielGesungen = z < stufen;
            double bester = kMinusUnendlich;
            int woher = 0;
            for (int d = -sprung; d <= sprung; ++d)
            {
                const int i = j - d;  // die Ausgangsstufe: j = i + d
                if (i < 0 || i >= stufen) continue;
                const double dreieck = logDreieck[static_cast<std::size_t>(d + sprung)] - logNorm[static_cast<std::size_t>(i)];
                for (int vorherGesungen = 0; vorherGesungen < 2; ++vorherGesungen)
                {
                    const int quelle = vorherGesungen == 1 ? i : i + stufen;
                    const double wert = delta[static_cast<std::size_t>(quelle)] + dreieck +
                                        ((vorherGesungen == 1) == zielGesungen ? logBleibt : logWechselt);
                    if (wert > bester)
                    {
                        bester = wert;
                        woher = quelle;
                    }
                }
            }
            neu[static_cast<std::size_t>(z)] = bester + beob[static_cast<std::size_t>(z)];
            von[static_cast<std::size_t>(z)] = static_cast<std::int16_t>(woher);
        }
        std::swap(delta, neu);
    }

    // zurueck vom besten Endzustand
    verlauf.hz.assign(static_cast<std::size_t>(rahmen), 0.0f);
    if (rahmen == 0) return verlauf;
    int z = static_cast<int>(std::max_element(delta.begin(), delta.end()) - delta.begin());
    for (std::int64_t k = rahmen - 1; k >= 0; --k)
    {
        if (z < stufen)
        {
            // gesungen: der Kandidat dieser Stufe mit dem groessten Gewicht, mit seiner verfeinerten Frequenz
            double p = -1.0;
            for (const PYinKandidat& c : kandidaten[static_cast<std::size_t>(k)])
                if (stufeVon(c.hz) == z && c.p > p)
                {
                    p = c.p;
                    verlauf.hz[static_cast<std::size_t>(k)] = static_cast<float>(c.hz);
                }
        }
        if (k > 0) z = zurueck[static_cast<std::size_t>(k)][static_cast<std::size_t>(z)];
    }
    return verlauf;
}

}  // namespace lumi::loop
