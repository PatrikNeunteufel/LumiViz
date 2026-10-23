/**
 * @file Statistik.hpp
 * @brief Der statistische Teil des Koerpers oberhalb der gerechneten Moden: die Groessen aus
 *        den gerechneten Moden geschaetzt, die Moden daraus mit festem Samen gezogen
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S146–S148: KB1 a,
 *        PL31c-5 a, ST1 a; Stufe 31c-5c)
 *
 * **Groessen (ST1 a)** aus dem oberen Band der gerechneten Moden (obere Haelfte des Fensters,
 * beim Steinway 2,5–5 kHz): die Modendichte n(f) als Gerade, ueber das Ende fortgesetzt (eine
 * fallende Gerade wird waagrecht — die Dichte einer Platte faellt nicht); je Stegpunkt die
 * Streuung der Formwerte (quadratisches Mittel); je Hoerort der Betrag des Abstrahlgewichts
 * (quadratisches Mittel — dieselbe Leistung wie die gerechneten Moden). Die Daempfung folgt
 * demselben Gesetz wie die gerechneten Moden (RT-0425 Gl. 1), damit es an der Grenze keine Naht
 * gibt.
 *
 * **Ziehung** wie Boutillon & Ege 2013: f_{ν+1} = f_ν + r / n(f_ν) mit r nach Rayleigh (Gl. 23,
 * 24: p(x) = πx/2 · exp(−πx²/4), Mittel 1); je Stegpunkt ein Formwert normal verteilt mit
 * seiner Streuung, je Hoerort ein Gewicht mit seinem Betrag und gleichverteilter Phase. Die
 * Stegpunkte sind unabhaengig voneinander (ST1 a).
 *
 * **Zufall** mit eigenem Generator (SplitMix64) und eigenen Umrechnungen — dieselbe Folge auf
 * jedem Compiler und jeder Standardbibliothek (std::*_distribution legt die Folge nicht fest).
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek.
 */
#pragma once

#include "InstrumentBody/Auswertung.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <numbers>
#include <string>
#include <vector>

namespace lumi::koerper
{

/// Die Groessen des statistischen Teils (was die Koerperdatei traegt).
struct StatistischerTeil
{
    double vonHz = 0.0;   ///< hier beginnt er (die obere Grenze der gerechneten Moden)
    double bisHz = 0.0;   ///< hier endet er
    double dichte = 0.0;  ///< n(vonHz), Moden je Hz
    double anstieg = 0.0;  ///< dn/df, Moden je Hz²; ≥ 0
    Daempfung daempfung;
    std::vector<double> stegStreuung;  ///< je Stegpunkt das quadratische Mittel der Formwerte, 1/√kg
    std::vector<double> hoerBetrag;    ///< je Hoerort das quadratische Mittel von |G|, √kg/m²
    std::uint64_t samen = 0;
    double bandVonHz = 0.0;  ///< woraus geschaetzt wurde (zur Auskunft)
    double bandBisHz = 0.0;
    int bandModen = 0;

    [[nodiscard]] double dichteBei(double f) const { return dichte + anstieg * (f - vonHz); }
};

/// SplitMix64 (Steele, Lea & Flood 2014) — klein, schnell, auf jedem Compiler gleich.
class Zufall
{
public:
    explicit Zufall(std::uint64_t samen) : m_zustand(samen) {}

    std::uint64_t naechste()
    {
        std::uint64_t z = (m_zustand += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }

    /// gleichverteilt in (0, 1) — nie genau 0 oder 1
    double gleich() { return (static_cast<double>(naechste() >> 11) + 0.5) * (1.0 / 9007199254740992.0); }

    /// Rayleigh mit Mittel 1 (Boutillon & Ege Gl. 24): Umkehr der Verteilung 1 − exp(−πx²/4)
    double rayleigh() { return std::sqrt(-4.0 * std::log(gleich()) / std::numbers::pi); }

    /// normal verteilt, Mittel 0, Streuung 1 (Box-Muller, je Aufruf ein Paar, eines verworfen); die
    /// beiden Zuege in getrennten Anweisungen — in einem Ausdruck legt C++ ihre Reihenfolge nicht fest
    double normal()
    {
        const double u = gleich();
        const double v = gleich();
        return std::sqrt(-2.0 * std::log(u)) * std::cos(2.0 * std::numbers::pi * v);
    }

private:
    std::uint64_t m_zustand;
};

/// Schaetzt die Groessen aus den gerechneten Moden (aufsteigend) im Band [bandVon, bandBis);
/// der Teil beginnt bei `vonHz` (der oberen Grenze des gerechneten Fensters) und endet bei `bisHz`.
/// Leer (dichte = 0), wenn im Band weniger als zehn Moden liegen.
[[nodiscard]] inline StatistischerTeil schaetze(const std::vector<ModeImKoerper>& gerechnet, double bandVon, double bandBis,
                                                double vonHz, double bisHz, Daempfung daempfung, std::uint64_t samen)
{
    StatistischerTeil st;
    st.vonHz = vonHz;
    st.bisHz = bisHz;
    st.daempfung = daempfung;
    st.samen = samen;
    st.bandVonHz = bandVon;
    st.bandBisHz = bandBis;
    std::vector<const ModeImKoerper*> band;
    for (const ModeImKoerper& m : gerechnet)
        if (m.frequenzHz >= bandVon && m.frequenzHz < bandBis) band.push_back(&m);
    st.bandModen = static_cast<int>(band.size());
    if (band.size() < 10 || !(bandBis > bandVon)) return st;

    // Dichte: Zaehlung in zehn Stuecken, eine Gerade durch die Mitten (kleinste Quadrate)
    constexpr int kStuecke = 10;
    const double breite = (bandBis - bandVon) / kStuecke;
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    for (int i = 0; i < kStuecke; ++i)
    {
        const double a = bandVon + i * breite, b = a + breite;
        const double zahl = static_cast<double>(std::count_if(band.begin(), band.end(), [&](const ModeImKoerper* m) {
            return m->frequenzHz >= a && m->frequenzHz < b;
        }));
        const double x = (a + b) / 2.0, y = zahl / breite;
        sx += x;
        sy += y;
        sxx += x * x;
        sxy += x * y;
    }
    const double nenner = kStuecke * sxx - sx * sx;
    const double steigung = (kStuecke * sxy - sx * sy) / nenner;
    const double achse = (sy - steigung * sx) / kStuecke;
    st.anstieg = std::max(0.0, steigung);
    st.dichte = steigung > 0.0 ? achse + steigung * vonHz : sy / kStuecke;

    // Streuung je Stegpunkt und Betrag je Hoerort, quadratisch gemittelt
    const std::size_t ns = band.front()->amStegpunkt.size(), nh = band.front()->abstrahlung.size();
    st.stegStreuung.assign(ns, 0.0);
    st.hoerBetrag.assign(nh, 0.0);
    for (const ModeImKoerper* m : band)
    {
        for (std::size_t j = 0; j < ns; ++j) st.stegStreuung[j] += m->amStegpunkt[j] * m->amStegpunkt[j];
        for (std::size_t h = 0; h < nh; ++h) st.hoerBetrag[h] += std::norm(m->abstrahlung[h]);
    }
    for (double& v : st.stegStreuung) v = std::sqrt(v / static_cast<double>(band.size()));
    for (double& v : st.hoerBetrag) v = std::sqrt(v / static_cast<double>(band.size()));
    return st;
}

/// Zieht die Moden des statistischen Teils, aufsteigend, in [vonHz, bisHz). Derselbe Teil gibt
/// immer dieselben Moden.
[[nodiscard]] inline std::vector<ModeImKoerper> ziehe(const StatistischerTeil& st)
{
    std::vector<ModeImKoerper> moden;
    if (!(st.dichte > 0.0) || !(st.bisHz > st.vonHz)) return moden;
    Zufall zufall(st.samen);
    double f = st.vonHz;
    for (;;)
    {
        f += zufall.rayleigh() / st.dichteBei(f);
        if (f >= st.bisHz) break;
        ModeImKoerper m;
        m.frequenzHz = f;
        m.verlustfaktor = st.daempfung.verlustfaktor(f);
        for (const double s : st.stegStreuung) m.amStegpunkt.push_back(s * zufall.normal());
        for (const double b : st.hoerBetrag)
        {
            const double phase = 2.0 * std::numbers::pi * zufall.gleich();
            m.abstrahlung.push_back(std::polar(b, phase));
        }
        moden.push_back(std::move(m));
    }
    return moden;
}

}  // namespace lumi::koerper
