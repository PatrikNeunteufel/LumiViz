/**
 * @file Luftraum.hpp
 * @brief Der Luftraum hinter der Platte: die Moden der Platte und die Luft in den Loechern, gekoppelt
 *        ueber den einen Druck eines Hohlraums (docs/audio/Instrumente_IC_Konzept.md §6: F-K1 a,
 *        GZ4 a, Entscheid Patrik S196: KQ8 a; Stufe 34k-5)
 *
 * **Modell** wie die IC-Vorlage `harvest/instruments-calculation/rechnung/gemeinsam/Koerper` (dort nach
 * Wright 1996, Gl. A.12–A.28, dem Muster des Gitarrenkorpus): jede Mode der Platte wirkt als Kolben
 * mit dem Volumen je Auslenkung a_k = ∫ φ_k dS, die Luft in den Loechern als ein Kolben mit der
 * akustischen Masse M_a; alle haengen nur ueber den Druck P = −α (Σ a_k q_k + u) zusammen,
 * α = ρ c² / V. Anders als dort kommen die Moden hier aus der Rechnung der Platte **ohne Luft**
 * (im Vakuum, M-orthonormal: modale Masse 1) — die Steife je Mode ist also ω_k², ohne Abzug.
 *
 *     q̈_k + ω_k² q_k = a_k P + φ_k(Steg) F          (je Mode der Platte)
 *     M_a ü          = P                             (die Luft aller Loecher, u in m³ nach aussen)
 *
 * **Mehrere Loecher** liegen parallel am selben Druck: je Loch die Masse ρ A_j L_j mit
 * L_j = Plattendicke + Muendungskorrektur · Radius (flaechengleicher Kreis); zusammen
 * 1/M_a = Σ A_j / (ρ L_j). Ihr Volumenfluss teilt sich im Verhaeltnis A_j / L_j. (Ein Kolben je Loch
 * gaebe dieselben Moden und dazu Moden der Frequenz 0 — Luft, die durch ein Loch hinein und durch
 * das andere hinaus stroemt, ohne den Druck zu aendern.)
 *
 * **Gerechnet** wird das ungedaempfte System genau: mit y = (q, √M_a u) ist die Steifigkeit
 * diag(ω_k², 0) + α z zᵀ, z = (a_k, 1/√M_a) — eine Diagonale plus Rang 1. Ihre Eigenwerte sind die
 * Nullstellen der Saekulargleichung 1 + α Σ z_i² / (d_i − λ) = 0, je eine zwischen zwei
 * benachbarten d_i (und eine ueber dem groessten); die Eigenvektoren v_i ∝ z_i / (d_i − λ). Keine
 * fremde Bibliothek. Moden ohne Volumen (a_k = 0: links und rechts gegengleich) bleiben, wie sie sind.
 *
 * **Grenzen:** nur der Druck des Hohlraums als Ganzes (Helmholtz) — keine hoeheren Moden der Luft im
 * Kasten; die Waende ausser der Platte starr; die Luft vor der Platte nur als Muendungskorrektur.
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek.
 */
#pragma once

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/Bauplan.hpp"
#include "InstrumentBody/Eigenloeser.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <utility>
#include <vector>

namespace lumi::koerper
{

// --- Diagonale plus Rang 1 -----------------------------------------------------------

/// Eigenwerte und Eigenvektoren von diag(d) + alpha · z zᵀ (alpha > 0).
struct RangEins
{
    std::vector<double> werte;                 ///< aufsteigend
    std::vector<std::vector<double>> vektoren;  ///< je Eigenwert ein Vektor der Laenge n, Norm 1
};

[[nodiscard]] inline RangEins eigenRangEins(const std::vector<double>& d, const std::vector<double>& z, double alpha)
{
    const std::size_t n = d.size();
    RangEins aus;
    if (n == 0 || z.size() != n) return aus;

    // ein Platz = ein Basisvektor aus den urspruenglichen Achsen (nach dem Zusammenlegen gleicher d
    // eine Mischung zweier Achsen)
    struct Platz
    {
        double d = 0.0;
        double z = 0.0;
        std::vector<std::pair<std::size_t, double>> basis;
    };
    std::vector<Platz> plaetze(n);
    double groesstes = 0.0, zz = 0.0;
    for (std::size_t i = 0; i < n; ++i)
    {
        plaetze[i] = {d[i], z[i], {{i, 1.0}}};
        groesstes = std::max(groesstes, std::abs(d[i]));
        zz += z[i] * z[i];
    }
    std::stable_sort(plaetze.begin(), plaetze.end(), [](const Platz& a, const Platz& b) { return a.d < b.d; });
    const double toleranz = 1e-14 * std::max(groesstes, alpha * zz);

    // gleiche d zusammenlegen: (z_i e_i + z_j e_j)/r koppelt mit r, (z_j e_i − z_i e_j)/r gar nicht
    std::vector<Platz> fertig;  // ohne Kopplung: Eigenwert d, Eigenvektor die Basis
    std::vector<Platz> aktiv;
    for (Platz& p : plaetze)
    {
        if (alpha * p.z * p.z <= toleranz)
        {
            fertig.push_back(std::move(p));
            continue;
        }
        if (!aktiv.empty() && p.d - aktiv.back().d <= toleranz)
        {
            Platz& a = aktiv.back();
            const double r = std::hypot(a.z, p.z);
            Platz frei;
            frei.d = p.d;
            for (const auto& [i, c] : a.basis) frei.basis.push_back({i, c * p.z / r});
            for (const auto& [i, c] : p.basis) frei.basis.push_back({i, -c * a.z / r});
            std::vector<std::pair<std::size_t, double>> neu;
            for (const auto& [i, c] : a.basis) neu.push_back({i, c * a.z / r});
            for (const auto& [i, c] : p.basis) neu.push_back({i, c * p.z / r});
            a.basis = std::move(neu);
            a.z = r;
            fertig.push_back(std::move(frei));
            continue;
        }
        aktiv.push_back(std::move(p));
    }

    struct Paar
    {
        double wert = 0.0;
        std::vector<double> vektor;
    };
    std::vector<Paar> paare;
    for (const Platz& p : fertig)
    {
        Paar e{p.d, std::vector<double>(n, 0.0)};
        for (const auto& [i, c] : p.basis) e.vektor[i] += c;
        paare.push_back(std::move(e));
    }

    const std::size_t m = aktiv.size();
    double zzAktiv = 0.0;
    for (const Platz& p : aktiv) zzAktiv += p.z * p.z;
    for (std::size_t k = 0; k < m; ++k)
    {
        // die Nullstelle zwischen d_k und d_{k+1} (die letzte: bis d_m + alpha·Σz²)
        const double luecke = k + 1 < m ? aktiv[k + 1].d - aktiv[k].d : alpha * zzAktiv;
        // f(λ) = 1 + alpha Σ z_j² / (d_j − λ), mit λ = d_bezug + versatz; die Differenzen d_j − d_bezug genau
        const auto f = [&](std::size_t bezug, double versatz) {
            double summe = 1.0;
            for (std::size_t j = 0; j < m; ++j) summe += alpha * aktiv[j].z * aktiv[j].z / ((aktiv[j].d - aktiv[bezug].d) - versatz);
            return summe;
        };
        // von dem Pol aus zaehlen, dem die Nullstelle naeher liegt (f waechst in der Luecke von −∞ nach +∞)
        const bool vonUnten = k + 1 >= m || f(k, luecke / 2.0) > 0.0;
        const std::size_t bezug = vonUnten ? k : k + 1;
        double lo = vonUnten ? 0.0 : -luecke / 2.0, hi = vonUnten ? (k + 1 < m ? luecke / 2.0 : luecke) : 0.0;
        for (int i = 0; i < 200; ++i)
        {
            const double mitte = (lo + hi) / 2.0;
            if (mitte == lo || mitte == hi) break;
            if (f(bezug, mitte) > 0.0) hi = mitte;
            else lo = mitte;
        }
        const double versatz = (lo + hi) / 2.0;
        Paar e{aktiv[bezug].d + versatz, std::vector<double>(n, 0.0)};
        std::vector<double> v(m);
        double norm = 0.0;
        for (std::size_t j = 0; j < m; ++j)
        {
            v[j] = aktiv[j].z / ((aktiv[j].d - aktiv[bezug].d) - versatz);
            norm += v[j] * v[j];
        }
        norm = std::sqrt(norm);
        for (std::size_t j = 0; j < m; ++j)
            for (const auto& [i, c] : aktiv[j].basis) e.vektor[i] += c * v[j] / norm;
        paare.push_back(std::move(e));
    }
    std::stable_sort(paare.begin(), paare.end(), [](const Paar& a, const Paar& b) { return a.wert < b.wert; });
    for (Paar& p : paare)
    {
        aus.werte.push_back(p.wert);
        aus.vektoren.push_back(std::move(p.vektor));
    }
    return aus;
}

// --- der Hohlraum aus dem Bauplan ----------------------------------------------------

/// Die Luft in einem Loch.
struct LochLuft
{
    Punkt2 mitte;           ///< Schwerpunkt des Lochs
    double flaeche = 0.0;   ///< m²
    double laenge = 0.0;    ///< m: Plattendicke + Muendungskorrektur · Radius
    double anteil = 0.0;    ///< ihr Anteil am Volumenfluss aller Loecher (A/L, zusammen 1)
};

/// Was die Kopplung braucht: α = ρ c² / V und die akustische Masse der Loecher zusammen.
struct LuftraumGroessen
{
    double alpha = 0.0;            ///< Pa/m³
    double akustischeMasse = 0.0;  ///< kg/m⁴; 0 = kein Loch (der Hohlraum ist zu)
    std::vector<LochLuft> loecher;

    /// Die Helmholtz-Frequenz bei starren Waenden in Hz; 0 ohne Loch.
    [[nodiscard]] double helmholtzHz() const
    {
        return akustischeMasse > 0.0 ? std::sqrt(alpha / akustischeMasse) / (2.0 * std::numbers::pi) : 0.0;
    }
};

/// Der Schwerpunkt eines geschlossenen Umrisses.
[[nodiscard]] inline Punkt2 schwerpunkt(const std::vector<Punkt2>& umriss)
{
    double doppelt = 0.0, sx = 0.0, sy = 0.0;
    const std::size_t n = umriss.size();
    for (std::size_t k = 0; k < n; ++k)
    {
        const Punkt2& a = umriss[k];
        const Punkt2& b = umriss[(k + 1) % n];
        const double kreuz = a.x * b.y - b.x * a.y;
        doppelt += kreuz;
        sx += (a.x + b.x) * kreuz;
        sy += (a.y + b.y) * kreuz;
    }
    if (doppelt == 0.0) return n > 0 ? umriss[0] : Punkt2{};
    return {sx / (3.0 * doppelt), sy / (3.0 * doppelt)};
}

[[nodiscard]] inline LuftraumGroessen luftraumGroessen(const Bauplan& plan, double dichte, double schall)
{
    LuftraumGroessen g;
    if (!plan.hohlraum.an()) return g;
    g.alpha = dichte * schall * schall / plan.hohlraum.volumen;
    double leitwert = 0.0;  // Σ A / (ρ L)
    for (const auto& loch : plan.platte.loecher)
    {
        LochLuft l;
        l.mitte = schwerpunkt(loch);
        l.flaeche = flaeche(loch);
        const double radius = std::sqrt(l.flaeche / std::numbers::pi);
        l.laenge = plan.platte.dickeBei(l.mitte) + plan.hohlraum.muendung * radius;
        if (l.flaeche <= 0.0 || l.laenge <= 0.0) continue;
        l.anteil = l.flaeche / l.laenge;
        leitwert += l.flaeche / (dichte * l.laenge);
        g.loecher.push_back(l);
    }
    double summe = 0.0;
    for (const LochLuft& l : g.loecher) summe += l.anteil;
    for (LochLuft& l : g.loecher) l.anteil /= summe;
    g.akustischeMasse = leitwert > 0.0 ? 1.0 / leitwert : 0.0;
    return g;
}

// --- die Kopplung -----------------------------------------------------------------------

/// Eine Mode des gekoppelten Systems.
struct LuftraumMode
{
    double frequenzHz = 0.0;
    std::vector<double> platte;  ///< je Mode der Platte ihr Anteil (die Form ist Σ platte_k · Form_k)
    double oeffnung = 0.0;       ///< u je Einheit der Mode: das Volumen, das durch alle Loecher nach aussen tritt
    double luftAnteil = 0.0;     ///< Anteil der Bewegungsenergie in der Luft der Loecher, 0 … 1
};

/// Koppelt die Moden der Platte (Frequenzen im Vakuum, `volumen` a_k = ∫ φ_k dS mit M-orthonormalen
/// Formen) ueber den Hohlraum. Zurueck so viele Moden wie hinein, mit einem Loch eine mehr; aufsteigend.
[[nodiscard]] inline std::vector<LuftraumMode> koppleLuftraum(const std::vector<double>& frequenzenHz, const std::vector<double>& volumen,
                                                              const LuftraumGroessen& g)
{
    std::vector<LuftraumMode> aus;
    const std::size_t n = frequenzenHz.size();
    if (volumen.size() != n || g.alpha <= 0.0) return aus;
    const bool offen = g.akustischeMasse > 0.0;
    std::vector<double> d, z;
    for (std::size_t k = 0; k < n; ++k)
    {
        const double omega = 2.0 * std::numbers::pi * frequenzenHz[k];
        d.push_back(omega * omega);
        z.push_back(volumen[k]);
    }
    if (offen)
    {
        d.push_back(0.0);
        z.push_back(1.0 / std::sqrt(g.akustischeMasse));
    }
    const RangEins e = eigenRangEins(d, z, g.alpha);
    for (std::size_t m = 0; m < e.werte.size(); ++m)
    {
        LuftraumMode mode;
        mode.frequenzHz = std::sqrt(std::max(0.0, e.werte[m])) / (2.0 * std::numbers::pi);
        mode.platte.assign(e.vektoren[m].begin(), e.vektoren[m].begin() + static_cast<std::ptrdiff_t>(n));
        if (offen)
        {
            const double y = e.vektoren[m][n];
            mode.oeffnung = y / std::sqrt(g.akustischeMasse);
            mode.luftAnteil = y * y;
        }
        aus.push_back(std::move(mode));
    }
    return aus;
}

// --- der Koerper mit Luftraum ----------------------------------------------------------

/// Was die Loecher an einem Hoerort beitragen, wenn durch sie das Volumen `oeffnung` (je Einheit der
/// Mode) nach aussen tritt: je Loch eine kleine Quelle in seinem Schwerpunkt, in derselben Schreibweise
/// wie das Rayleigh-Integral der Platte, ρ/(2π) · u_j · e^{−ikR}/R.
[[nodiscard]] inline std::vector<std::complex<double>> lochAbstrahlung(const LuftraumGroessen& g, const std::vector<Hoerort>& hoerorte,
                                                                       Luft luft, double frequenzHz, double oeffnung)
{
    std::vector<std::complex<double>> aus(hoerorte.size(), {0.0, 0.0});
    const double k = 2.0 * std::numbers::pi * frequenzHz / luft.schall;
    for (std::size_t h = 0; h < hoerorte.size(); ++h)
    {
        const Hoerort& o = hoerorte[h];
        for (const LochLuft& l : g.loecher)
        {
            const double r = std::sqrt((l.mitte.x - o.ort.x) * (l.mitte.x - o.ort.x) + (l.mitte.y - o.ort.y) * (l.mitte.y - o.ort.y) + o.hoehe * o.hoehe);
            aus[h] += luft.dichte / (2.0 * std::numbers::pi) * oeffnung * l.anteil / r * std::complex<double>(std::cos(-k * r), std::sin(-k * r));
        }
    }
    return aus;
}

/// Die Moden eines Koerpers mit Luftraum.
struct MitLuftraum
{
    std::vector<ModeImKoerper> moden;  ///< aufsteigend
    std::vector<Mode> formen;          ///< dieselben Moden als Form der Platte (ohne die Luft), gleiche Reihenfolge
    std::vector<double> luftAnteil;    ///< je Mode der Anteil der Bewegungsenergie in der Luft der Loecher
};

/// Koppelt die gerechneten Moden der Platte (im Vakuum, M-orthonormal) ueber den Hohlraum des Bauplans
/// und wertet jede gekoppelte Mode aus: Wert an den Stegpunkten und Abstrahlung der Platte aus der
/// gemischten Form (`Auswertung::werte` bei der neuen Frequenz), dazu die Loecher als Quellen; der
/// Verlustfaktor ist der der Platte mal ihrem Anteil plus Anteil der Luft / Guete.
[[nodiscard]] inline MitLuftraum mitLuftraum(const Auswertung& auswertung, const Bauplan& plan, Luft luft, const std::vector<Mode>& platte)
{
    MitLuftraum aus;
    const LuftraumGroessen g = luftraumGroessen(plan, luft.dichte, luft.schall);
    std::vector<double> hz, volumen;
    for (const Mode& m : platte)
    {
        hz.push_back(m.frequenzHz);
        volumen.push_back(auswertung.volumen(m.form));
    }
    for (const LuftraumMode& lm : koppleLuftraum(hz, volumen, g))
    {
        Mode mode;
        mode.frequenzHz = lm.frequenzHz;
        mode.form.assign(platte.empty() ? 0 : platte[0].form.size(), 0.0);
        for (std::size_t k = 0; k < platte.size(); ++k)
        {
            const double c = lm.platte[k];
            if (std::abs(c) < 1e-14) continue;
            for (std::size_t i = 0; i < mode.form.size(); ++i) mode.form[i] += c * platte[k].form[i];
        }
        ModeImKoerper mk = auswertung.werte(mode);
        mk.verlustfaktor = (1.0 - lm.luftAnteil) * mk.verlustfaktor + lm.luftAnteil / plan.hohlraum.guete;
        mk.luftAnteil = lm.luftAnteil;
        const auto loch = lochAbstrahlung(g, plan.hoerorte, luft, lm.frequenzHz, lm.oeffnung);
        for (std::size_t h = 0; h < mk.abstrahlung.size() && h < loch.size(); ++h) mk.abstrahlung[h] += loch[h];
        aus.moden.push_back(std::move(mk));
        aus.formen.push_back(std::move(mode));
        aus.luftAnteil.push_back(lm.luftAnteil);
    }
    return aus;
}

}  // namespace lumi::koerper
