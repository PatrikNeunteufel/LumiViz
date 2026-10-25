/**
 * @file Antwort.hpp
 * @brief Die Antwort des Bodens: Schalldruck am Hoerort je Kraft am Stegpunkt, aus den Moden
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S149: AW1 a, FT1 a, PG1 a;
 *        Stufe 31c-7)
 *
 * Je Mode n (bezueglich M normiert) ist die modale Koordinate q_n die Antwort eines Einmassen-
 * schwingers auf φ_n(A)·F, und der Druck am Hoerort p = Σ q̈_n G_n (`Auswertung.hpp`). Im Spektrum:
 *
 *     H(ω) = Σ_n φ_n(A) G_n · (−ω²) / (ω_n² − ω² + j η_n ω_n ω)      in Pa/N
 *
 * Die Antwort wird nach AW1 a direkt im Spektrum gebildet, an den Frequenzpunkten der FFT, mit der
 * sie danach gefaltet wird (`antwortSpektrum`). Die Rueckwandlung in eine Zeitreihe gibt die
 * Koeffizienten des Filters je Abtastwert: IDFT(H(ω_k)) = T · h(kT).
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek.
 */
#pragma once

#include "InstrumentBody/Auswertung.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <vector>

namespace lumi::koerper
{

/// H bei `frequenzHz` fuer eine Kraft am Stegpunkt `stegpunkt` und den Druck am Hoerort `hoerort`, Pa/N.
[[nodiscard]] inline std::complex<double> antwort(const std::vector<ModeImKoerper>& moden, std::size_t stegpunkt,
                                                  std::size_t hoerort, double frequenzHz)
{
    const double w = 2.0 * std::numbers::pi * frequenzHz;
    std::complex<double> summe{0.0, 0.0};
    for (const ModeImKoerper& m : moden)
    {
        if (stegpunkt >= m.amStegpunkt.size() || hoerort >= m.abstrahlung.size()) continue;
        const double wn = 2.0 * std::numbers::pi * m.frequenzHz;
        summe += m.amStegpunkt[stegpunkt] * m.abstrahlung[hoerort] * (-w * w) /
                 std::complex<double>(wn * wn - w * w, m.verlustfaktor * wn * w);
    }
    return summe;
}

/// H an den n/2 + 1 Frequenzpunkten einer FFT der Laenge n bei der Abtastrate `rate` (f_k = k·rate/n),
/// in Pa/N, jede Mode an jedem Punkt — der Bezug fuer `antwortSpektrum` (AW1 a; seit AW3-1 a nur noch
/// in Tests und zum Vergleich).
[[nodiscard]] inline std::vector<std::complex<double>> antwortSpektrumExakt(const std::vector<ModeImKoerper>& moden,
                                                                            std::size_t stegpunkt, std::size_t hoerort,
                                                                            double rate, std::size_t n)
{
    // Real- und Imaginaerteil getrennt und die Division von Hand: a·(−ω²)/(d_r + j d_i)
    // = −ω²/(d_r² + d_i²) · a·(d_r − j d_i) — so laesst sich die innere Schleife vektorisieren
    // (die Division zweier std::complex prueft auf Unendlich und NaN)
    const std::size_t bins = n / 2 + 1;
    std::vector<double> w(bins), w2(bins), re(bins, 0.0), im(bins, 0.0);
    const double dw = 2.0 * std::numbers::pi * rate / static_cast<double>(n);
    for (std::size_t k = 0; k < bins; ++k)
    {
        w[k] = dw * static_cast<double>(k);
        w2[k] = w[k] * w[k];
    }
    for (const ModeImKoerper& m : moden)
    {
        if (stegpunkt >= m.amStegpunkt.size() || hoerort >= m.abstrahlung.size()) continue;
        const std::complex<double> a = m.amStegpunkt[stegpunkt] * m.abstrahlung[hoerort];
        const double ar = a.real(), ai = a.imag();
        const double wn = 2.0 * std::numbers::pi * m.frequenzHz;
        const double wn2 = wn * wn;
        const double daempfung = m.verlustfaktor * wn;
        for (std::size_t k = 1; k < bins; ++k)  // bei 0 Hz ist H = 0
        {
            const double dr = wn2 - w2[k];
            const double di = daempfung * w[k];
            const double t = -w2[k] / (dr * dr + di * di);
            re[k] += t * (ar * dr + ai * di);
            im[k] += t * (ai * dr - ar * di);
        }
    }
    std::vector<std::complex<double>> h(bins);
    for (std::size_t k = 0; k < bins; ++k) h[k] = {re[k], im[k]};
    return h;
}

/// Die Stellgroessen von `antwortSpektrum` (AW3-1 a), in Frequenzpunkten der FFT bzw. Rasterschritten.
struct RasterAntwort
{
    int schritt = 64;       ///< Abstand der Rasterpunkte in Frequenzpunkten
    double schmalAb = 16.0; ///< schmal = Halbwertsbreite η·f unter so vielen Rasterschritten: der Kern exakt
    int kern = 32;          ///< halbe Breite des exakten Kerns: so viele Rasterschritte plus zwei Halbwertsbreiten
    int uebergang = 16;     ///< so viele Rasterschritte geht das Gewicht von 1 auf 0 (Kosinus)
};

/// H wie `antwortSpektrumExakt`, schnell (AW3-1 a, S151): die Summe der Moden auf einem Raster je
/// `schritt` Frequenzpunkte, dazwischen kubisch interpoliert (Lagrange, vier Stuetzpunkte). Eine Resonanz,
/// die fuer das Raster zu schmal ist, wird mit einem weichen Gewicht w geteilt: w·H exakt an den feinen
/// Punkten ihres Kerns, (1 − w)·H aufs Raster — dort ist sie glatt. Breite Resonanzen (die hohen Moden)
/// kommen ganz aufs Raster. Geprueft gegen die exakte Rechnung (`test_LoopKoerperKlang`).
[[nodiscard]] inline std::vector<std::complex<double>> antwortSpektrum(const std::vector<ModeImKoerper>& moden,
                                                                       std::size_t stegpunkt, std::size_t hoerort,
                                                                       double rate, std::size_t n,
                                                                       const RasterAntwort& r = {})
{
    const std::size_t bins = n / 2 + 1;
    const double dw = 2.0 * std::numbers::pi * rate / static_cast<double>(n);
    const auto s = static_cast<std::size_t>(std::max(1, r.schritt));
    // Rasterpunkte c = −1 … C bei k = c·s; Platz c + 1 — je ein Punkt vor 0 und zwei hinter dem letzten fuer die Interpolation
    const std::size_t plaetze = (bins - 1) / s + 4;
    std::vector<double> rr(plaetze, 0.0), ri(plaetze, 0.0), fr(bins, 0.0), fi(bins, 0.0);
    std::vector<double> wc(plaetze), wc2(plaetze);
    for (std::size_t p = 0; p < plaetze; ++p)
    {
        wc[p] = dw * static_cast<double>(static_cast<std::ptrdiff_t>(p * s) - static_cast<std::ptrdiff_t>(s));
        wc2[p] = wc[p] * wc[p];
    }
    const double kernS = static_cast<double>(r.kern) * static_cast<double>(s);
    const double uebergangS = std::max(1.0, static_cast<double>(r.uebergang) * static_cast<double>(s));
    // das Gewicht des Kerns im Abstand d (Frequenzpunkte) von der Resonanz
    const auto gewicht = [&](double d, double innen) {
        if (d <= innen) return 1.0;
        if (d >= innen + uebergangS) return 0.0;
        return 0.5 * (1.0 + std::cos(std::numbers::pi * (d - innen) / uebergangS));
    };
    for (const ModeImKoerper& m : moden)
    {
        if (stegpunkt >= m.amStegpunkt.size() || hoerort >= m.abstrahlung.size()) continue;
        const std::complex<double> a = m.amStegpunkt[stegpunkt] * m.abstrahlung[hoerort];
        const double ar = a.real(), ai = a.imag();
        const double wn = 2.0 * std::numbers::pi * m.frequenzHz;
        const double wn2 = wn * wn;
        const double daempfung = m.verlustfaktor * wn;
        const double breite = daempfung / dw;  // Halbwertsbreite in Frequenzpunkten
        const double kn = wn / dw;
        // H einer Mode bei ω (dieselbe Rechnung wie in `antwortSpektrumExakt`)
        const auto beitrag = [&](double w, double w2, double& re, double& im) {
            const double dr = wn2 - w2;
            const double di = daempfung * w;
            const double t = -w2 / (dr * dr + di * di);
            re = t * (ar * dr + ai * di);
            im = t * (ai * dr - ar * di);
        };
        double re = 0.0, im = 0.0;
        if (breite >= r.schmalAb * static_cast<double>(s))
        {
            for (std::size_t p = 0; p < plaetze; ++p)
            {
                beitrag(wc[p], wc2[p], re, im);
                rr[p] += re;
                ri[p] += im;
            }
            continue;
        }
        // schmal: der Kern exakt, der Rest aufs Raster
        const double innen = kernS + 2.0 * breite;
        for (std::size_t p = 0; p < plaetze; ++p)
        {
            const double g = 1.0 - gewicht(std::abs(wc[p] / dw - kn), innen);
            if (g == 0.0) continue;
            beitrag(wc[p], wc2[p], re, im);
            rr[p] += g * re;
            ri[p] += g * im;
        }
        const double rand = innen + uebergangS;
        const auto von = static_cast<std::size_t>(std::max(1.0, std::ceil(kn - rand)));
        const auto bis = static_cast<std::size_t>(std::min(static_cast<double>(bins - 1), std::floor(kn + rand)));
        for (std::size_t k = von; k <= bis && k < bins; ++k)
        {
            const double w = dw * static_cast<double>(k);
            const double g = gewicht(std::abs(static_cast<double>(k) - kn), innen);
            if (g == 0.0) continue;
            beitrag(w, w * w, re, im);
            fr[k] += g * re;
            fi[k] += g * im;
        }
    }
    // interpolieren: k = c·s + j, Stuetzpunkte c − 1 … c + 2 (Plaetze c … c + 3), Gewichte je j einmal gerechnet
    std::vector<std::array<double, 4>> lagrange(s);
    for (std::size_t j = 0; j < s; ++j)
    {
        const double t = static_cast<double>(j) / static_cast<double>(s);
        lagrange[j] = {-t * (t - 1.0) * (t - 2.0) / 6.0, (t + 1.0) * (t - 1.0) * (t - 2.0) / 2.0,
                       -(t + 1.0) * t * (t - 2.0) / 2.0, (t + 1.0) * t * (t - 1.0) / 6.0};
    }
    std::vector<std::complex<double>> h(bins);
    for (std::size_t k = 1; k < bins; ++k)  // bei 0 Hz ist H = 0
    {
        const std::size_t c = k / s;
        const std::array<double, 4>& l = lagrange[k % s];
        double re = fr[k], im = fi[k];
        for (std::size_t q = 0; q < 4; ++q)
        {
            re += l[q] * rr[c + q];
            im += l[q] * ri[c + q];
        }
        h[k] = {re, im};
    }
    return h;
}

/// So lang klingt die Antwort: bis die langsamste Mode um 60 dB gefallen ist (Amplitude
/// e^(−η ω_n t / 2)) — ohne Obergrenze (AW2 a, S151: die 8 s aus AW1 a schnitten einen Boden aus Glas
/// ab, und was ueber die Periode der FFT reichte, faltete sich auf den Anfang zurueck). Eine FFT von
/// mindestens dieser Laenge laesst nur Reste unter −60 dB zurueckfalten.
[[nodiscard]] inline double antwortLaengeS(const std::vector<ModeImKoerper>& moden)
{
    double laengste = 0.0;
    for (const ModeImKoerper& m : moden)
    {
        const double sigma = 0.5 * m.verlustfaktor * 2.0 * std::numbers::pi * m.frequenzHz;
        if (sigma > 0.0) laengste = std::max(laengste, std::log(1000.0) / sigma);
    }
    return laengste;
}

}  // namespace lumi::koerper
