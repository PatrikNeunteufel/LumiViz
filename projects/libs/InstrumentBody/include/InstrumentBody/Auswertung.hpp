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

// `Daempfung` (das Gesetz α(f) = a f² + b f + c) steht seit HU4-1 a (S160) im Bauplan: `Bauplan.hpp`.

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

// `Abtastung`, `bezug`, `abtastung` und `wertBei` stehen seit KQ46 a (S210) in `Platte.hpp`: auch die Punktfedern
// des Bauplans brauchen sie.

/// Was der Koerper je Mode traegt (KB1 a).
struct ModeImKoerper
{
    double frequenzHz = 0.0;
    double verlustfaktor = 0.0;                      ///< η
    double luftAnteil = 0.0;                         ///< Anteil der Bewegungsenergie in der Luft (F-K1 a); 0 = ohne Hohlraum
    /// Davon der Anteil der Luft laengs im Rohr (KQ59 a, KQ63 a, S215); 0 = ein Druck. Der Rest von `luftAnteil` ist
    /// die Luft in den Loechern und Oeffnungen. Seit Fassung 7 der Koerperdatei dort als `tubeShare`.
    double rohrAnteil = 0.0;
    /// Der Verlustfaktor, den die Luft der Mode gibt (KQ71 a, S218): η = (1 − luftAnteil) · η der Platte + luftVerlust.
    /// Seit Fassung 8 der Koerperdatei dort als `airLoss`; 0 = ohne Hohlraum.
    double luftVerlust = 0.0;
    std::vector<double> werkstoffAnteil;            ///< U_m / U je Werkstoff (Reihenfolge des Bauplans), DM1 a; leer = nicht gerechnet
    /// Anteil der Verformungsenergie in den Federn an Punkten (KQ49 a, S210), 0 … 1; 0 = keine Federn. Seit Fassung 6
    /// der Koerperdatei dort als `springShare` (KQ53 a, S211). `werkstoffAnteil` bleibt auf die Werkstoffe normiert
    /// (Summe 1).
    double federAnteil = 0.0;
    std::vector<double> amStegpunkt;                 ///< φ je Stegpunkt (Reihenfolge des Bauplans), 1/√kg
    std::vector<std::complex<double>> abstrahlung;  ///< G je Hoerort (Reihenfolge des Bauplans), √kg/m²
    /// KQ35-2 a (S206): die Richtung, in der die Mode den Steg bewegt — Winkel gegen die Senkrechte der Decke in Grad,
    /// in der Ebene quer zu den Saiten; 0 = senkrecht (jeder Koerper aus einem Bauplan), ±90 = nur parallel zur Decke.
    /// `amStegpunkt` ist der Formwert laengs dieser Richtung: senkrecht φ·cos, parallel φ·sin.
    double stegWinkelGrad = 0.0;

    /// Der Formwert senkrecht zur Decke am Stegpunkt `i` — was eine Kraft senkrecht zur Decke anregt. Bei Winkel 0
    /// Bit fuer Bit `amStegpunkt[i]`.
    [[nodiscard]] double senkrechtAm(std::size_t i) const
    {
        return stegWinkelGrad == 0.0 ? amStegpunkt[i] : amStegpunkt[i] * std::cos(stegWinkelGrad * std::numbers::pi / 180.0);
    }
    /// Der Formwert parallel zur Decke (quer zu den Saiten) am Stegpunkt `i`.
    [[nodiscard]] double parallelAm(std::size_t i) const
    {
        return stegWinkelGrad == 0.0 ? 0.0 : amStegpunkt[i] * std::sin(stegWinkelGrad * std::numbers::pi / 180.0);
    }

    [[nodiscard]] bool operator==(const ModeImKoerper&) const = default;
};

/// Bereitet die Auswertung einmal vor (Abtastung der Stegpunkte, Gauss-Punkte mit ihrem Abstand
/// zu jedem Hoerort) und wertet dann jede Mode aus. Mit `energie` (DM1 a) bekommt jede Mode ihre
/// Anteile der Verformungsenergie je Werkstoff und daraus ihren Verlustfaktor
/// (`verlustfaktorAusAnteilen` mit `werkstoffVerlust` je Werkstoff, 0 = das Gesetz).
class Auswertung
{
public:
    /// Mit `rueckseite.an()` und einem Umriss (dem der Platte) traegt jeder Gauss-Punkt zusaetzlich die
    /// Rueckseite: Weg zum naechsten Randpunkt, zweimal die Randtiefe, dann zum Hoerort; umgekehrtes
    /// Vorzeichen, in `werte` oberhalb `schattenHz` abgeschattet (HU4 a). Ohne sie wie bisher, Bit fuer Bit.
    Auswertung(const PlattenSystem& s, const Plattenelement& element, const std::vector<Stegpunkt>& stegpunkte,
               const std::vector<Hoerort>& hoerorte, Luft luft = {}, Daempfung daempfung = {},
               const Verformungsenergie* energie = nullptr, std::vector<double> werkstoffVerlust = {},
               const std::vector<Punkt2>& umriss = {}, Rueckseite rueckseite = {})
        : m_luft(luft), m_daempfung(daempfung), m_energie(energie), m_werkstoffVerlust(std::move(werkstoffVerlust)),
          m_nk(element.knoten()), m_rueckseite(rueckseite), m_rueck(rueckseite.an() && umriss.size() >= 3)
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
        if (m_rueck)
        {
            m_abstandRueck.assign(hoerorte.size(), {});
            m_gewichtRueck.assign(hoerorte.size(), {});
            for (auto& v : m_abstandRueck) v.reserve(np);
            for (auto& v : m_gewichtRueck) v.reserve(np);
        }
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
                m_flaeche.push_back(w * det);
                m_y.push_back(p.y);
                const Randpunkt rand = m_rueck ? naechsterRandpunkt(umriss, p) : Randpunkt{};
                for (std::size_t h = 0; h < hoerorte.size(); ++h)
                {
                    const Hoerort& o = hoerorte[h];
                    const double r = std::sqrt((p.x - o.ort.x) * (p.x - o.ort.x) + (p.y - o.ort.y) * (p.y - o.ort.y) + o.hoehe * o.hoehe);
                    m_abstand[h].push_back(r);
                    m_gewicht[h].push_back(w * det / r);
                    if (m_rueck)
                    {
                        const Punkt2& q = rand.punkt;
                        const double rr = rand.abstand + 2.0 * m_rueckseite.randtiefe +
                                          std::sqrt((q.x - o.ort.x) * (q.x - o.ort.x) + (q.y - o.ort.y) * (q.y - o.ort.y) + o.hoehe * o.hoehe);
                        m_abstandRueck[h].push_back(rr);
                        m_gewichtRueck[h].push_back(w * det / rr);
                    }
                }
            }
        }
    }

    /// Die Federn an Punkten des Bauplans (KQ49 a, S210): danach traegt jede Mode `federAnteil`, und ihr
    /// Verlustfaktor ist η = (1 − Σ a_f) · η_Platte + Σ a_f η_f, mit a_f = U_f / (U_Platte + Σ U_f) und
    /// U_f = ½ k w(p)². U_Platte kommt aus `energie`; ohne sie gilt ½ (2πf)² abzueglich der Federn — das stimmt
    /// nur fuer eine bezueglich M normierte Form der Platte allein. Ohne Federn rechnet `werte` wie bisher,
    /// Bit fuer Bit.
    void setzePunktfedern(const PlattenSystem& s, const Plattenelement& element, const std::vector<Punktfeder>& federn)
    {
        m_federn.clear();
        for (const Punktfeder& f : federn)
            if (f.steife > 0.0) m_federn.push_back({abtastung(s, element, f.ort), f.steife, f.verlustfaktor});
    }

    /// Die Abtastung jedes Stegpunkts (Reihenfolge des Bauplans) — zur Pruefung, ob er im Netz liegt.
    [[nodiscard]] const std::vector<Abtastung>& stegpunkte() const { return m_steg; }

    /// Das Volumen, das eine Form verdraengt: ∫ w dS ueber die Platte (fuer den Luftraum, `Luftraum.hpp`).
    [[nodiscard]] double volumen(const std::vector<double>& form) const
    {
        double summe = 0.0;
        for (std::size_t p = 0; p < m_flaeche.size(); ++p)
        {
            double w = 0.0;
            const std::size_t basis = p * static_cast<std::size_t>(m_nk);
            for (int i = 0; i < m_nk; ++i)
            {
                const int g = m_groesse[basis + static_cast<std::size_t>(i)];
                if (g >= 0) w += m_ansatz[basis + static_cast<std::size_t>(i)] * form[static_cast<std::size_t>(g)];
            }
            summe += w * m_flaeche[p];
        }
        return summe;
    }

    /// Das Volumen einer Form je Abschnitt laengs y (KQ59 a, fuer das Rohr in `Luftraum.hpp`): Abschnitt z reicht
    /// von `von` + z · `schritt` bis eins weiter; ein Gauss-Punkt davor oder dahinter zaehlt zum ersten bzw. letzten.
    /// Die Summe ist `volumen(form)` bis auf die Rundung.
    [[nodiscard]] std::vector<double> volumenLaengs(const std::vector<double>& form, double von, double schritt, int abschnitte) const
    {
        std::vector<double> aus(static_cast<std::size_t>(std::max(abschnitte, 0)), 0.0);
        if (aus.empty() || !(schritt > 0.0)) return aus;
        for (std::size_t p = 0; p < m_flaeche.size(); ++p)
        {
            double w = 0.0;
            const std::size_t basis = p * static_cast<std::size_t>(m_nk);
            for (int i = 0; i < m_nk; ++i)
            {
                const int g = m_groesse[basis + static_cast<std::size_t>(i)];
                if (g >= 0) w += m_ansatz[basis + static_cast<std::size_t>(i)] * form[static_cast<std::size_t>(g)];
            }
            const int z = std::clamp(static_cast<int>(std::floor((m_y[p] - von) / schritt)), 0, abschnitte - 1);
            aus[static_cast<std::size_t>(z)] += w * m_flaeche[p];
        }
        return aus;
    }

    [[nodiscard]] ModeImKoerper werte(const Mode& mode) const
    {
        ModeImKoerper aus;
        aus.frequenzHz = mode.frequenzHz;
        if (m_energie != nullptr) aus.werkstoffAnteil = m_energie->anteile(mode.form);
        aus.verlustfaktor = verlustfaktorAusAnteilen(m_daempfung, mode.frequenzHz, aus.werkstoffAnteil, m_werkstoffVerlust);
        if (!m_federn.empty())
        {
            // KQ49 a: die Energie in den Federn traegt deren Verlustfaktor, der Rest den der Platte
            double uFedern = 0.0, verlust = 0.0;
            for (const Feder& f : m_federn)
            {
                const double w = wertBei(f.ort, mode.form);
                const double u = 0.5 * f.steife * w * w;
                uFedern += u;
                verlust += u * f.verlustfaktor;
            }
            double uPlatte = 0.0;
            if (m_energie != nullptr)
                for (const double u : m_energie->jeWerkstoff(mode.form)) uPlatte += u;
            else
            {
                const double omega = 2.0 * std::numbers::pi * mode.frequenzHz;
                uPlatte = std::max(0.0, 0.5 * omega * omega - uFedern);
            }
            if (const double summe = uPlatte + uFedern; summe > 0.0)
            {
                aus.federAnteil = uFedern / summe;
                aus.verlustfaktor = (1.0 - aus.federAnteil) * aus.verlustfaktor + verlust / summe;
            }
        }
        for (const Abtastung& a : m_steg) aus.amStegpunkt.push_back(wertBei(a, mode.form));

        const double k = 2.0 * std::numbers::pi * mode.frequenzHz / m_luft.schall;
        // die Rueckseite: umgekehrtes Vorzeichen, oberhalb schattenHz mit erster Ordnung abgeschattet
        const std::complex<double> schatten =
            m_rueckseite.schattenHz > 0.0 ? 1.0 / std::complex<double>(1.0, mode.frequenzHz / m_rueckseite.schattenHz)
                                          : std::complex<double>(1.0, 0.0);
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
                if (m_rueck)
                {
                    const double phaseRueck = -k * m_abstandRueck[h][p];
                    summe[h] -= schatten * (w * m_gewichtRueck[h][p]) * std::complex<double>(std::cos(phaseRueck), std::sin(phaseRueck));
                }
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
    struct Feder
    {
        Abtastung ort;
        double steife = 0.0;
        double verlustfaktor = 0.0;
    };
    std::vector<Feder> m_federn;                ///< die Federn an Punkten (KQ49 a); leer = keine
    std::vector<int> m_groesse;                ///< je Gauss-Punkt nk Plaetze von w (−1 = gehalten)
    std::vector<double> m_ansatz;               ///< je Gauss-Punkt nk Ansatzwerte
    std::vector<double> m_flaeche;              ///< je Gauss-Punkt Gewicht · |det J| (seine Flaeche)
    std::vector<double> m_y;                    ///< je Gauss-Punkt seine Lage y (KQ59 a: das Rohr laengs y)
    std::vector<std::vector<double>> m_abstand;  ///< je Hoerort je Gauss-Punkt R
    std::vector<std::vector<double>> m_gewicht;  ///< je Hoerort je Gauss-Punkt Gewicht · |det J| / R
    Rueckseite m_rueckseite;
    bool m_rueck = false;                             ///< Rueckseite an und ein Umriss da
    std::vector<std::vector<double>> m_abstandRueck;  ///< je Hoerort je Gauss-Punkt R' ueber den Rand
    std::vector<std::vector<double>> m_gewichtRueck;  ///< je Hoerort je Gauss-Punkt Gewicht · |det J| / R'
};

}  // namespace lumi::koerper
