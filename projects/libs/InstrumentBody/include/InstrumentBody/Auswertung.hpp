/**
 * @file Auswertung.hpp
 * @brief Was der Koerper je Mode traegt: der Wert der Form an jedem Stegpunkt, die
 *        Daempfung und je Hoerort das Abstrahlgewicht nach Rayleigh
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S146–S148: KB1 a, BM6 a,
 *        PL31c-5 a; Stufe 31c-5b)
 *
 * **Am Stegpunkt** der Wert φ(x) der Form (Auslenkung w, bezueglich M normiert, 1/√kg): die
 * modale Masse dort ist 1/φ², das Vorzeichen sagt, wie zwei Stegpunkte zueinander schwingen.
 * Gerechnet ueber die Ansatzfunktionen des Elements, in dem der Punkt liegt (`abtastung`).
 *
 * **Daempfung** je Werkstoff nach dem Anteil der Verformungsenergie (DM1 a, `Verformungsenergie.hpp`);
 * ein Werkstoff ohne eigenen Verlustfaktor — und jeder, wenn keine Anteile gerechnet sind — folgt
 * dem Gesetz des Koerpers, modal wie Chabassier, Joly & Chaigne 2013 Gl. 17 (Ẍ + α(f) Ẋ + (2πf)² X = F),
 * α(f) = a f² + b f + c in 1/s mit f in Hz, a, b, c nach RT-0425 Gl. 1. Gelesen als Polynom in f:
 * so trifft der Verlustfaktor η = α/(2πf) die gemessenen 1–3 % (in ω gelesen gaeben dieselben
 * Zahlen 8–70 %). Die Form αλ + β√λ + γ in λ = ω² steht bei Chabassier, Duruflé & Joly 2015
 * Teil 2, S. 6.
 *
 * **Abstrahlung** nach Rayleigh, die Platte in einer Schallwand (BM6 a): Druck am Hoerort
 * p(t) = ρa/(2π) ∫ ẅ(x, t − R/c)/R dS. Fuer die Mode n mit w = φ q ist p = q̈ · G, mit
 * G = ρa/(2π) ∫ φ e^{−iωR/c}/R dS bei ihrer Frequenz — ueber die schmale Bandbreite einer Mode
 * aendert sich das Integral kaum (PL31c-5 a). Einheit √kg/m² (Pa je modaler Beschleunigung).
 * Integriert ueber die Gauss-Punkte aller Elemente (k + 1 je Richtung); R von der Mittelflaeche
 * der Platte (z = 0) zum Hoerort. Luft nach RT-0425 §2.2.
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek.
 */
#pragma once

#include "InstrumentBody/Bauplan.hpp"
#include "InstrumentBody/Eigenloeser.hpp"
#include "InstrumentBody/Platte.hpp"
#include "InstrumentBody/Verformungsenergie.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <numbers>
#include <utility>
#include <vector>

namespace lumi::koerper
{

/// Die Luft (RT-0425 §2.2).
struct Luft
{
    double dichte = 1.29;   ///< kg/m³
    double schall = 340.0;  ///< m/s

    [[nodiscard]] bool operator==(const Luft&) const = default;
};

/// Die modale Daempfung α(f) = a f² + b f + c in 1/s, f in Hz (RT-0425 Gl. 1).
struct Daempfung
{
    double a = 2.0e-5;
    double b = 0.07;
    double c = 0.0;

    [[nodiscard]] double alpha(double frequenzHz) const { return a * frequenzHz * frequenzHz + b * frequenzHz + c; }
    /// Verlustfaktor η = α / (2π f).
    [[nodiscard]] double verlustfaktor(double frequenzHz) const
    {
        return frequenzHz > 0.0 ? alpha(frequenzHz) / (2.0 * std::numbers::pi * frequenzHz) : 0.0;
    }

    [[nodiscard]] bool operator==(const Daempfung&) const = default;
};

/// Der Verlustfaktor einer Mode aus ihren Anteilen der Verformungsenergie je Werkstoff (DM1 a):
/// ein Werkstoff mit eigenem Verlustfaktor (> 0) traegt ihn, einer ohne (0) das Gesetz des
/// Koerpers. Gerechnet als η = η_Gesetz + Σ_{η_m > 0} a_m (η_m − η_Gesetz) — ohne eigene
/// Verlustfaktoren genau das Gesetz, Bit fuer Bit. Ohne Anteile ebenso.
[[nodiscard]] inline double verlustfaktorAusAnteilen(const Daempfung& gesetz, double frequenzHz, const std::vector<double>& anteile,
                                                     const std::vector<double>& werkstoffVerlust)
{
    const double eta = gesetz.verlustfaktor(frequenzHz);
    double aus = eta;
    for (std::size_t m = 0; m < anteile.size() && m < werkstoffVerlust.size(); ++m)
        if (werkstoffVerlust[m] > 0.0) aus += anteile[m] * (werkstoffVerlust[m] - eta);
    return aus;
}

/// Wo ein Punkt im System liegt: welche freien Groessen w ihn mit welchem Gewicht tragen.
struct Abtastung
{
    int element = -1;            ///< −1 = kein Element
    bool imNetz = false;         ///< false = ausserhalb, auf das naechste Element geklemmt
    double abstand = 0.0;        ///< m, vom Punkt zum Netz (0 im Netz)
    std::vector<int> groessen;   ///< Plaetze in K und M (nur w, nur freie)
    std::vector<double> gewichte;
};

/// Die Lage von `p` im Dreieck mit diesen Ecken in Bezugskoordinaten (ξ, η).
[[nodiscard]] inline std::array<double, 2> bezug(const std::array<Punkt2, 3>& e, Punkt2 p)
{
    const double j11 = e[1].x - e[0].x, j12 = e[2].x - e[0].x, j21 = e[1].y - e[0].y, j22 = e[2].y - e[0].y;
    const double det = j11 * j22 - j12 * j21;
    const double dx = p.x - e[0].x, dy = p.y - e[0].y;
    return {(j22 * dx - j12 * dy) / det, (-j21 * dx + j11 * dy) / det};
}

/// Wie `p` aus den freien Groessen w des Systems folgt. Liegt `p` in keinem Element, gilt das
/// naechste (der Punkt auf seinen Rand geklemmt; `imNetz` = false, `abstand` > 0).
[[nodiscard]] inline Abtastung abtastung(const PlattenSystem& s, const Plattenelement& element, Punkt2 p)
{
    Abtastung a;
    double bestAbstand = std::numeric_limits<double>::infinity();
    std::array<double, 2> bestXiEta{};
    for (std::size_t t = 0; t < s.elemente.size(); ++t)
    {
        const auto& ek = s.elemente[t];
        const std::array<Punkt2, 3> e{s.knoten[ek[0]], s.knoten[ek[1]], s.knoten[ek[2]]};
        auto [xi, eta] = bezug(e, p);
        constexpr double kSpiel = 1e-12;
        if (xi >= -kSpiel && eta >= -kSpiel && xi + eta <= 1.0 + kSpiel)
        {
            a.element = static_cast<int>(t);
            bestAbstand = 0.0;
            bestXiEta = {xi, eta};
            break;
        }
        // geklemmt in das Dreieck, dann der Abstand
        xi = std::max(0.0, xi);
        eta = std::max(0.0, eta);
        if (xi + eta > 1.0)
        {
            const double summe = xi + eta;
            xi /= summe;
            eta /= summe;
        }
        const Punkt2 q{e[0].x + (e[1].x - e[0].x) * xi + (e[2].x - e[0].x) * eta,
                       e[0].y + (e[1].y - e[0].y) * xi + (e[2].y - e[0].y) * eta};
        const double d = std::hypot(q.x - p.x, q.y - p.y);
        if (d < bestAbstand)
        {
            bestAbstand = d;
            bestXiEta = {xi, eta};
            a.element = static_cast<int>(t);
        }
    }
    if (a.element < 0) return a;
    a.imNetz = bestAbstand == 0.0;
    a.abstand = bestAbstand;
    std::vector<double> n, dxi, deta;
    element.basis(bestXiEta[0], bestXiEta[1], n, dxi, deta);
    const auto& ek = s.elemente[static_cast<std::size_t>(a.element)];
    for (std::size_t i = 0; i < ek.size(); ++i)
    {
        const int g = s.frei[static_cast<std::size_t>(3 * ek[i])];  // w des Knotens
        if (g < 0 || n[i] == 0.0) continue;
        a.groessen.push_back(g);
        a.gewichte.push_back(n[i]);
    }
    return a;
}

/// Der Wert einer Form (nummeriert wie K und M) an der abgetasteten Stelle.
[[nodiscard]] inline double wertBei(const Abtastung& a, const std::vector<double>& form)
{
    double w = 0.0;
    for (std::size_t i = 0; i < a.groessen.size(); ++i) w += a.gewichte[i] * form[static_cast<std::size_t>(a.groessen[i])];
    return w;
}

/// Was der Koerper je Mode traegt (KB1 a).
struct ModeImKoerper
{
    double frequenzHz = 0.0;
    double verlustfaktor = 0.0;                      ///< η
    std::vector<double> werkstoffAnteil;             ///< U_m / U je Werkstoff (Reihenfolge des Bauplans), DM1 a; leer = nicht gerechnet
    std::vector<double> amStegpunkt;                 ///< φ je Stegpunkt (Reihenfolge des Bauplans), 1/√kg
    std::vector<std::complex<double>> abstrahlung;  ///< G je Hoerort (Reihenfolge des Bauplans), √kg/m²

    [[nodiscard]] bool operator==(const ModeImKoerper&) const = default;
};

/// Bereitet die Auswertung einmal vor (Abtastung der Stegpunkte, Gauss-Punkte mit ihrem Abstand
/// zu jedem Hoerort) und wertet dann jede Mode aus. Mit `energie` (DM1 a) bekommt jede Mode ihre
/// Anteile der Verformungsenergie je Werkstoff und daraus ihren Verlustfaktor
/// (`verlustfaktorAusAnteilen` mit `werkstoffVerlust` je Werkstoff, 0 = das Gesetz).
class Auswertung
{
public:
    Auswertung(const PlattenSystem& s, const Plattenelement& element, const std::vector<Stegpunkt>& stegpunkte,
               const std::vector<Hoerort>& hoerorte, Luft luft = {}, Daempfung daempfung = {},
               const Verformungsenergie* energie = nullptr, std::vector<double> werkstoffVerlust = {})
        : m_luft(luft), m_daempfung(daempfung), m_energie(energie), m_werkstoffVerlust(std::move(werkstoffVerlust)),
          m_nk(element.knoten())
    {
        for (const Stegpunkt& sp : stegpunkte) m_steg.push_back(abtastung(s, element, sp.ort));

        // Gauss-Punkte: je Punkt die w-Groessen der Knoten (−1 = gehalten) und ihre Ansatzwerte
        const auto punkte = dreiecksPunkte(element.ordnung() + 1);
        const std::size_t np = s.elemente.size() * punkte.size();
        m_groesse.reserve(np * static_cast<std::size_t>(m_nk));
        m_ansatz.reserve(np * static_cast<std::size_t>(m_nk));
        m_abstand.assign(hoerorte.size(), {});
        m_gewicht.assign(hoerorte.size(), {});
        for (auto& v : m_abstand) v.reserve(np);
        for (auto& v : m_gewicht) v.reserve(np);
        std::vector<double> n, dxi, deta;
        std::vector<std::vector<double>> ansatz;
        for (const auto& [xi, eta, w] : punkte)
        {
            element.basis(xi, eta, n, dxi, deta);
            ansatz.push_back(n);
        }
        for (const auto& ek : s.elemente)
        {
            const Punkt2 e0 = s.knoten[ek[0]], e1 = s.knoten[ek[1]], e2 = s.knoten[ek[2]];
            const double det = std::abs((e1.x - e0.x) * (e2.y - e0.y) - (e2.x - e0.x) * (e1.y - e0.y));
            for (std::size_t g = 0; g < punkte.size(); ++g)
            {
                const auto& [xi, eta, w] = punkte[g];
                for (int i = 0; i < m_nk; ++i)
                {
                    m_groesse.push_back(s.frei[static_cast<std::size_t>(3 * ek[static_cast<std::size_t>(i)])]);
                    m_ansatz.push_back(ansatz[g][static_cast<std::size_t>(i)]);
                }
                const Punkt2 p{e0.x + (e1.x - e0.x) * xi + (e2.x - e0.x) * eta, e0.y + (e1.y - e0.y) * xi + (e2.y - e0.y) * eta};
                for (std::size_t h = 0; h < hoerorte.size(); ++h)
                {
                    const Hoerort& o = hoerorte[h];
                    const double r = std::sqrt((p.x - o.ort.x) * (p.x - o.ort.x) + (p.y - o.ort.y) * (p.y - o.ort.y) + o.hoehe * o.hoehe);
                    m_abstand[h].push_back(r);
                    m_gewicht[h].push_back(w * det / r);
                }
            }
        }
    }

    /// Die Abtastung jedes Stegpunkts (Reihenfolge des Bauplans) — zur Pruefung, ob er im Netz liegt.
    [[nodiscard]] const std::vector<Abtastung>& stegpunkte() const { return m_steg; }

    [[nodiscard]] ModeImKoerper werte(const Mode& mode) const
    {
        ModeImKoerper aus;
        aus.frequenzHz = mode.frequenzHz;
        if (m_energie != nullptr) aus.werkstoffAnteil = m_energie->anteile(mode.form);
        aus.verlustfaktor = verlustfaktorAusAnteilen(m_daempfung, mode.frequenzHz, aus.werkstoffAnteil, m_werkstoffVerlust);
        for (const Abtastung& a : m_steg) aus.amStegpunkt.push_back(wertBei(a, mode.form));

        const double k = 2.0 * std::numbers::pi * mode.frequenzHz / m_luft.schall;
        std::vector<std::complex<double>> summe(m_abstand.size(), {0.0, 0.0});
        const std::size_t np = m_abstand.empty() ? 0 : m_abstand[0].size();
        for (std::size_t p = 0; p < np; ++p)
        {
            double w = 0.0;
            const std::size_t basis = p * static_cast<std::size_t>(m_nk);
            for (int i = 0; i < m_nk; ++i)
            {
                const int g = m_groesse[basis + static_cast<std::size_t>(i)];
                if (g >= 0) w += m_ansatz[basis + static_cast<std::size_t>(i)] * mode.form[static_cast<std::size_t>(g)];
            }
            if (w == 0.0) continue;
            for (std::size_t h = 0; h < summe.size(); ++h)
            {
                const double phase = -k * m_abstand[h][p];
                summe[h] += w * m_gewicht[h][p] * std::complex<double>(std::cos(phase), std::sin(phase));
            }
        }
        const double faktor = m_luft.dichte / (2.0 * std::numbers::pi);
        for (const auto& s : summe) aus.abstrahlung.push_back(faktor * s);
        return aus;
    }

private:
    Luft m_luft;
    Daempfung m_daempfung;
    const Verformungsenergie* m_energie = nullptr;
    std::vector<double> m_werkstoffVerlust;
    int m_nk = 0;
    std::vector<Abtastung> m_steg;
    std::vector<int> m_groesse;                 ///< je Gauss-Punkt nk Plaetze von w (−1 = gehalten)
    std::vector<double> m_ansatz;               ///< je Gauss-Punkt nk Ansatzwerte
    std::vector<std::vector<double>> m_abstand;  ///< je Hoerort je Gauss-Punkt R
    std::vector<std::vector<double>> m_gewicht;  ///< je Hoerort je Gauss-Punkt Gewicht · |det J| / R
};

}  // namespace lumi::koerper
