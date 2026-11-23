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
 * **Das Rohr** (seit Bauplan-Fassung 10; KQ59 a, KQ61 a–KQ65 a, S215; Herleitung und Probe
 * `docs/modellierung/gezupfte-saite/proben/2026-11-23_S214_harfe_rohr.py`, Harfenbericht §22–§23): ist der Hohlraum
 * lang gegen die Wellenlaenge (Harfe: 1,3 m, erste Laengsmode bei geschlossenen Oeffnungen um 150 Hz), haengt der
 * Druck von y ab. Dann gilt S/(ρc²) p_tt − (S p_y)_y / ρ = −(Quelle je Laenge), entwickelt nach den Moden ψ_j des
 * beidseitig geschlossenen Rohrs mit dem Querschnitt S(y) (ψ_0 = 1, Σ S ψ² Δy = V). Je Rohrmode j ≥ 1 eine Hilfsgroesse
 * s_j mit der Masse α/ω_j²; je Loch und Oeffnung ein Luftpfropfen u_i mit seiner Masse ρ L_i / A_i an seinem Ort y_i:
 *
 *     P_j = −α (D_j − s_j),   D_j = Σ_k B_jk q_k + Σ_i ψ_j(y_i) u_i,   B_jk = ∫ φ_k ψ_j(y) dS     (s_0 = 0)
 *     U = ½ Σ ω_k² q_k² + ½ α Σ_j (D_j − s_j)²,   T = ½ Σ q̇_k² + ½ Σ M_i u̇_i² + ½ Σ (α/ω_j²) ṡ_j²
 *
 * Mit j = 0 allein ist das der eine Druck oben (dort die Loecher zu einem Pfropfen zusammengelegt). Gerechnet wird
 * die dichte symmetrische Eigenaufgabe mit `kern::zerlegeSymmetrisch` aus `InstrumentCore` (KQ65 a); die Rohrmoden
 * aus finiten Volumen laengs y (`kRohrAbschnitteJeWelle` je Wellenlaenge an der oberen Grenze), alle bis zur oberen
 * Grenze des Fensters (KQ62 a). Moden der Frequenz 0 (Luft, die durch ein Loch hinein und ein anderes hinaus stroemt,
 * ohne den Druck zu aendern) fallen weg. Der Verlust (KQ63 a): Anteil der Luft in den Pfropfen / `Hohlraum::guete`
 * plus Anteil der Luft laengs / `Rohr::guete`. **Grenzen des Rohrs:** keine Quermoden (die Wellenlaenge gross gegen
 * die Breite), der Druck an einem Loch der an seinem Schwerpunkt; mit allen Oeffnungen der Harfe liegen die
 * Laengsmoden A2–A4 um 9–13 % unter Bell 1988 Kap. 4 (Ursache offen, KQ64 a).
 *
 * Reines C++ ohne Qt; die Eigenzerlegung aus `InstrumentCore`.
 */
#pragma once

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/Bauplan.hpp"
#include "InstrumentBody/Eigenloeser.hpp"
#include "InstrumentCore/Eigenzerlegung.hpp"

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
    /// Hoehe ueber der Platte in m; 0 = ein Loch der Platte. Eine Oeffnung ausserhalb der Platte (KQ17 a)
    /// traegt hier ihre Lage.
    double hoehe = 0.0;
    /// Ein Loch der Platte liegt in der Schallwand der Platte und strahlt in den Halbraum (ρ/2π); eine
    /// Oeffnung ausserhalb strahlt frei in den ganzen Raum (ρ/4π) — die Schale schattet nicht ab.
    bool frei = false;
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
    // seit Fassung 5 (KQ17 a): Oeffnungen ausserhalb der Platte, parallel zu den Loechern
    for (const Oeffnung& o : plan.hohlraum.oeffnungen)
    {
        LochLuft l;
        l.mitte = o.ort;
        l.hoehe = o.hoehe;
        l.frei = true;
        l.flaeche = o.flaeche;
        const double radius = std::sqrt(std::max(0.0, l.flaeche) / std::numbers::pi);
        l.laenge = o.lippe + plan.hohlraum.muendung * radius;
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
    double luftAnteil = 0.0;     ///< Anteil der Bewegungsenergie in der Luft (Loecher und, im Rohr, laengs), 0 … 1
    // nur im Rohr (KQ59 a):
    double rohrAnteil = 0.0;          ///< davon der Anteil der Luft laengs
    std::vector<double> jeOeffnung;   ///< u_i je Loch und Oeffnung (Reihenfolge `LuftraumGroessen::loecher`)
    std::vector<double> druck;        ///< P_j je Rohrmode in Pa je Einheit der Mode; p(y) = Σ_j P_j ψ_j(y)
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

// --- das Rohr (KQ59 a) ------------------------------------------------------------------

/// Abschnitte je Wellenlaenge an der oberen Grenze des Fensters (die Rohrmoden liegen dort rund 0,4 % zu tief, darunter
/// mit dem Quadrat der Frequenz weniger) und die Grenzen ihrer Zahl (die Eigenzerlegung ist dicht).
inline constexpr int kRohrAbschnitteJeWelle = 20;
inline constexpr int kRohrAbschnitteMin = 64;
inline constexpr int kRohrAbschnitteMax = 600;

/// Die Moden des beidseitig geschlossenen Rohrs bei starren Waenden.
struct RohrModen
{
    double von = 0.0;      ///< y am Anfang des ersten Abschnitts
    double schritt = 0.0;  ///< m, Laenge eines Abschnitts
    int abschnitte = 0;
    std::vector<double> flaeche;            ///< S je Abschnitt in m², so gestellt, dass Σ S Δy = V
    std::vector<double> omega;              ///< je Rohrmode ω in 1/s; omega[0] = 0
    std::vector<std::vector<double>> psi;   ///< je Rohrmode je Abschnitt; psi[0] = 1, Σ S ψ_j² Δy = V
    bool vollstaendig = true;               ///< die Eigenzerlegung ist zusammengelaufen

    [[nodiscard]] int abschnittBei(double y) const
    {
        if (abschnitte <= 0 || !(schritt > 0.0)) return 0;
        return std::clamp(static_cast<int>(std::floor((y - von) / schritt)), 0, abschnitte - 1);
    }
    /// ω der Rohrmode j in Hz
    [[nodiscard]] double hz(std::size_t j) const { return omega[j] / (2.0 * std::numbers::pi); }
};

/// Die Rohrmoden bis `bisHz` (KQ62 a) aus finiten Volumen laengs y.
[[nodiscard]] inline RohrModen rohrModen(const Rohr& rohr, double volumen, double schall, double bisHz)
{
    RohrModen r;
    if (rohr.querschnitte.size() < 2 || !(volumen > 0.0) || !(schall > 0.0)) return r;
    r.von = rohr.querschnitte.front().y;
    const double laenge = rohr.querschnitte.back().y - r.von;
    if (!(laenge > 0.0)) return r;
    const double proWelle = static_cast<double>(kRohrAbschnitteJeWelle) * std::max(bisHz, 0.0) / schall;
    r.abschnitte = std::clamp(static_cast<int>(std::ceil(laenge * proWelle)), kRohrAbschnitteMin, kRohrAbschnitteMax);
    const int n = r.abschnitte;
    r.schritt = laenge / n;
    r.flaeche.resize(static_cast<std::size_t>(n));
    double v = 0.0;
    for (int z = 0; z < n; ++z)
    {
        r.flaeche[static_cast<std::size_t>(z)] = rohr.flaecheBei(r.von + (z + 0.5) * r.schritt);
        v += r.flaeche[static_cast<std::size_t>(z)] * r.schritt;
    }
    for (double& s : r.flaeche) s *= volumen / v;

    // K p = ω² m p: zwischen zwei Abschnitten der Leitwert S_grenz / Δy, die Masse S Δy / c²
    std::vector<double> masse(static_cast<std::size_t>(n));
    for (int z = 0; z < n; ++z) masse[static_cast<std::size_t>(z)] = r.flaeche[static_cast<std::size_t>(z)] * r.schritt / (schall * schall);
    std::vector<double> a(static_cast<std::size_t>(n) * static_cast<std::size_t>(n), 0.0);
    const auto bei = [&](int i, int j) -> double& { return a[static_cast<std::size_t>(i) * static_cast<std::size_t>(n) + static_cast<std::size_t>(j)]; };
    for (int z = 0; z + 1 < n; ++z)
    {
        const double g = std::sqrt(r.flaeche[static_cast<std::size_t>(z)] * r.flaeche[static_cast<std::size_t>(z + 1)]) / r.schritt;
        const double mz = masse[static_cast<std::size_t>(z)], mn = masse[static_cast<std::size_t>(z + 1)];
        bei(z, z) += g / mz;
        bei(z + 1, z + 1) += g / mn;
        bei(z, z + 1) -= g / std::sqrt(mz * mn);
        bei(z + 1, z) -= g / std::sqrt(mz * mn);
    }
    const kern::SymmetrischeZerlegung e = kern::zerlegeSymmetrisch(n, std::move(a));
    r.vollstaendig = e.vollstaendig;
    r.omega.push_back(0.0);
    r.psi.emplace_back(static_cast<std::size_t>(n), 1.0);
    const double grenze = 2.0 * std::numbers::pi * bisHz;
    for (int j = 1; j < n; ++j)
    {
        const double w = std::sqrt(std::max(0.0, e.werte[static_cast<std::size_t>(j)]));
        if (w > grenze) break;
        std::vector<double> psi(static_cast<std::size_t>(n));
        double norm = 0.0;
        for (int z = 0; z < n; ++z)
        {
            const double v = e.vektoren[static_cast<std::size_t>(z) * static_cast<std::size_t>(n) + static_cast<std::size_t>(j)] /
                             std::sqrt(masse[static_cast<std::size_t>(z)]);
            psi[static_cast<std::size_t>(z)] = v;
            norm += r.flaeche[static_cast<std::size_t>(z)] * r.schritt * v * v;
        }
        const double faktor = (psi[0] < 0.0 ? -1.0 : 1.0) * std::sqrt(volumen / norm);  // am Anfang positiv
        for (double& v : psi) v *= faktor;
        r.omega.push_back(w);
        r.psi.push_back(std::move(psi));
    }
    return r;
}

/// Unter dieser Frequenz gilt eine gekoppelte Mode des Rohrs als Durchfluss ohne Druck (Frequenz 0) und faellt weg.
inline constexpr double kRohrDurchflussHz = 1.0;

/// Koppelt die Moden der Platte (im Vakuum, M-orthonormal; `volumenLaengs` je Mode ihr Volumen je Abschnitt des
/// Rohrs) ueber das Rohr mit seinen Loechern und Oeffnungen (`g.loecher`, jedes fuer sich). Ohne Plattenmoden das
/// Rohr bei starren Waenden. Zurueck aufsteigend, ohne die Moden der Frequenz 0.
[[nodiscard]] inline std::vector<LuftraumMode> koppleRohr(const std::vector<double>& frequenzenHz,
                                                          const std::vector<std::vector<double>>& volumenLaengs,
                                                          const LuftraumGroessen& g, const RohrModen& r, double dichte)
{
    std::vector<LuftraumMode> aus;
    const std::size_t n = frequenzenHz.size();
    if (volumenLaengs.size() != n || g.alpha <= 0.0 || r.psi.empty()) return aus;
    const std::size_t no = g.loecher.size();
    const std::size_t nj = r.psi.size();  // Rohrmoden mit j = 0
    const std::size_t ns = nj - 1;        // Hilfsgroessen s_j, j ≥ 1
    const std::size_t gesamt = n + no + ns;

    // B_jk = Σ_z b_k(z) ψ_j(z), ψ_j an den Loechern
    std::vector<std::vector<double>> b(nj, std::vector<double>(n, 0.0));
    for (std::size_t k = 0; k < n; ++k)
        for (std::size_t j = 0; j < nj; ++j)
        {
            double s = 0.0;
            for (std::size_t z = 0; z < volumenLaengs[k].size() && z < r.psi[j].size(); ++z) s += volumenLaengs[k][z] * r.psi[j][z];
            b[j][k] = s;
        }
    std::vector<std::vector<double>> psiLoch(nj, std::vector<double>(no, 0.0));
    std::vector<double> masse(gesamt, 1.0);
    for (std::size_t i = 0; i < no; ++i)
    {
        const int z = r.abschnittBei(g.loecher[i].mitte.y);
        for (std::size_t j = 0; j < nj; ++j) psiLoch[j][i] = r.psi[j][static_cast<std::size_t>(z)];
        masse[n + i] = dichte * g.loecher[i].laenge / g.loecher[i].flaeche;
    }
    for (std::size_t j = 1; j < nj; ++j) masse[n + no + j - 1] = g.alpha / (r.omega[j] * r.omega[j]);

    // A = M^{-1/2} (diag(ω_k²) + α Σ_j c_j c_jᵀ) M^{-1/2}
    std::vector<double> wurzel(gesamt);
    for (std::size_t i = 0; i < gesamt; ++i) wurzel[i] = std::sqrt(masse[i]);
    std::vector<std::vector<double>> c(nj, std::vector<double>(gesamt, 0.0));
    for (std::size_t j = 0; j < nj; ++j)
    {
        for (std::size_t k = 0; k < n; ++k) c[j][k] = b[j][k] / wurzel[k];
        for (std::size_t i = 0; i < no; ++i) c[j][n + i] = psiLoch[j][i] / wurzel[n + i];
        if (j >= 1) c[j][n + no + j - 1] = -1.0 / wurzel[n + no + j - 1];
    }
    const int dim = static_cast<int>(gesamt);
    std::vector<double> a(gesamt * gesamt, 0.0);
    for (std::size_t k = 0; k < n; ++k)
    {
        const double w = 2.0 * std::numbers::pi * frequenzenHz[k];
        a[k * gesamt + k] = w * w;
    }
    for (std::size_t j = 0; j < nj; ++j)
        for (std::size_t p = 0; p < gesamt; ++p)
        {
            if (c[j][p] == 0.0) continue;
            const double cp = g.alpha * c[j][p];
            for (std::size_t q = 0; q < gesamt; ++q) a[p * gesamt + q] += cp * c[j][q];
        }
    const kern::SymmetrischeZerlegung e = kern::zerlegeSymmetrisch(dim, std::move(a));

    for (std::size_t m = 0; m < gesamt; ++m)
    {
        LuftraumMode mode;
        mode.frequenzHz = std::sqrt(std::max(0.0, e.werte[m])) / (2.0 * std::numbers::pi);
        if (mode.frequenzHz < kRohrDurchflussHz) continue;
        std::vector<double> y(gesamt), x(gesamt);
        for (std::size_t i = 0; i < gesamt; ++i)
        {
            y[i] = e.vektoren[i * gesamt + m];
            x[i] = y[i] / wurzel[i];
        }
        mode.platte.assign(y.begin(), y.begin() + static_cast<std::ptrdiff_t>(n));
        double pfropfen = 0.0, laengs = 0.0;
        for (std::size_t i = 0; i < no; ++i)
        {
            pfropfen += y[n + i] * y[n + i];
            mode.jeOeffnung.push_back(x[n + i]);
            mode.oeffnung += x[n + i];
        }
        for (std::size_t s = 0; s < ns; ++s) laengs += y[n + no + s] * y[n + no + s];
        mode.luftAnteil = pfropfen + laengs;
        mode.rohrAnteil = laengs;
        for (std::size_t j = 0; j < nj; ++j)
        {
            double d = 0.0;
            for (std::size_t k = 0; k < n; ++k) d += b[j][k] * x[k];
            for (std::size_t i = 0; i < no; ++i) d += psiLoch[j][i] * x[n + i];
            if (j >= 1) d -= x[n + no + j - 1];
            mode.druck.push_back(-g.alpha * d);
        }
        aus.push_back(std::move(mode));
    }
    return aus;
}

/// Wie `lochAbstrahlung`, aber je Loch sein eigenes Volumen (`jeLoch`, Reihenfolge `g.loecher`) — fuer das Rohr.
[[nodiscard]] inline std::vector<std::complex<double>> lochAbstrahlungJeLoch(const LuftraumGroessen& g, const std::vector<Hoerort>& hoerorte,
                                                                             Luft luft, double frequenzHz, const std::vector<double>& jeLoch)
{
    std::vector<std::complex<double>> aus(hoerorte.size(), {0.0, 0.0});
    const double k = 2.0 * std::numbers::pi * frequenzHz / luft.schall;
    for (std::size_t h = 0; h < hoerorte.size(); ++h)
    {
        const Hoerort& o = hoerorte[h];
        for (std::size_t i = 0; i < g.loecher.size() && i < jeLoch.size(); ++i)
        {
            const LochLuft& l = g.loecher[i];
            const double dz = l.frei ? o.hoehe - l.hoehe : o.hoehe;
            const double r = std::sqrt((l.mitte.x - o.ort.x) * (l.mitte.x - o.ort.x) + (l.mitte.y - o.ort.y) * (l.mitte.y - o.ort.y) + dz * dz);
            // ein Loch der Platte strahlt in den Halbraum (ρ/2π), eine Oeffnung ausserhalb in den ganzen Raum (ρ/4π)
            const double faktor = luft.dichte / ((l.frei ? 4.0 : 2.0) * std::numbers::pi);
            aus[h] += faktor * jeLoch[i] / r * std::complex<double>(std::cos(-k * r), std::sin(-k * r));
        }
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
            if (!l.frei)
            {
                // ein Loch der Platte — unveraendert (Bit fuer Bit wie bis Fassung 4)
                const double r = std::sqrt((l.mitte.x - o.ort.x) * (l.mitte.x - o.ort.x) + (l.mitte.y - o.ort.y) * (l.mitte.y - o.ort.y) + o.hoehe * o.hoehe);
                aus[h] += luft.dichte / (2.0 * std::numbers::pi) * oeffnung * l.anteil / r * std::complex<double>(std::cos(-k * r), std::sin(-k * r));
                continue;
            }
            // eine Oeffnung ausserhalb der Platte (KQ17 a): Punktquelle im freien Raum an ihrem Ort
            const double dz = o.hoehe - l.hoehe;
            const double r = std::sqrt((l.mitte.x - o.ort.x) * (l.mitte.x - o.ort.x) + (l.mitte.y - o.ort.y) * (l.mitte.y - o.ort.y) + dz * dz);
            aus[h] += luft.dichte / (4.0 * std::numbers::pi) * oeffnung * l.anteil / r * std::complex<double>(std::cos(-k * r), std::sin(-k * r));
        }
    }
    return aus;
}

/// Die Moden eines Koerpers mit Luftraum.
struct MitLuftraum
{
    std::vector<ModeImKoerper> moden;  ///< aufsteigend
    std::vector<Mode> formen;          ///< dieselben Moden als Form der Platte (ohne die Luft), gleiche Reihenfolge
    std::vector<double> luftAnteil;    ///< je Mode der Anteil der Bewegungsenergie in der Luft
    std::vector<double> rohrAnteil;    ///< davon laengs im Rohr (KQ59 a); leer bei einem Druck
    int rohrModen = 0;                 ///< die Zahl der Rohrmoden j ≥ 1 (KQ62 a); 0 bei einem Druck
};

/// Der Koerper mit Rohr (KQ59 a): wie `mitLuftraum`, die Luft laengs y.
[[nodiscard]] inline MitLuftraum mitLuftraumRohr(const Auswertung& auswertung, const Bauplan& plan, Luft luft, const std::vector<Mode>& platte,
                                                 double bisHz)
{
    MitLuftraum aus;
    const LuftraumGroessen g = luftraumGroessen(plan, luft.dichte, luft.schall);
    double bis = bisHz;
    if (!(bis > 0.0))
        for (const Mode& m : platte) bis = std::max(bis, m.frequenzHz);
    const RohrModen r = rohrModen(plan.hohlraum.rohr, plan.hohlraum.volumen, luft.schall, bis);
    aus.rohrModen = static_cast<int>(r.omega.size()) - 1;
    std::vector<double> hz;
    std::vector<std::vector<double>> laengs;
    for (const Mode& m : platte)
    {
        hz.push_back(m.frequenzHz);
        laengs.push_back(auswertung.volumenLaengs(m.form, r.von, r.schritt, r.abschnitte));
    }
    for (const LuftraumMode& lm : koppleRohr(hz, laengs, g, r, luft.dichte))
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
        // KQ63 a: die Luft in den Pfropfen traegt die Guete der Oeffnungen, die Luft laengs die des Rohrs
        mk.verlustfaktor = (1.0 - lm.luftAnteil) * mk.verlustfaktor + (lm.luftAnteil - lm.rohrAnteil) / plan.hohlraum.guete +
                           lm.rohrAnteil / plan.hohlraum.rohr.guete;
        mk.luftAnteil = lm.luftAnteil;
        mk.rohrAnteil = lm.rohrAnteil;
        const auto loch = lochAbstrahlungJeLoch(g, plan.hoerorte, luft, lm.frequenzHz, lm.jeOeffnung);
        for (std::size_t h = 0; h < mk.abstrahlung.size() && h < loch.size(); ++h) mk.abstrahlung[h] += loch[h];
        aus.moden.push_back(std::move(mk));
        aus.formen.push_back(std::move(mode));
        aus.luftAnteil.push_back(lm.luftAnteil);
        aus.rohrAnteil.push_back(lm.rohrAnteil);
    }
    return aus;
}

/// Koppelt die gerechneten Moden der Platte (im Vakuum, M-orthonormal) ueber den Hohlraum des Bauplans
/// und wertet jede gekoppelte Mode aus: Wert an den Stegpunkten und Abstrahlung der Platte aus der
/// gemischten Form (`Auswertung::werte` bei der neuen Frequenz), dazu die Loecher als Quellen; der
/// Verlustfaktor ist der der Platte mal ihrem Anteil plus Anteil der Luft / Guete. Ist der Hohlraum ein Rohr
/// (KQ59 a), rechnet `mitLuftraumRohr` mit allen Rohrmoden bis `bisHz` (0 = bis zur hoechsten Plattenmode).
[[nodiscard]] inline MitLuftraum mitLuftraum(const Auswertung& auswertung, const Bauplan& plan, Luft luft, const std::vector<Mode>& platte,
                                             double bisHz = 0.0)
{
    if (plan.hohlraum.alsRohr()) return mitLuftraumRohr(auswertung, plan, luft, platte, bisHz);
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
