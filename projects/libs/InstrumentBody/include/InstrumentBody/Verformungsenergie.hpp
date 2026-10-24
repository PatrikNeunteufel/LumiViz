/**
 * @file Verformungsenergie.hpp
 * @brief Die Verformungsenergie einer Mode je Werkstoff — woraus ihre Daempfung folgt
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheid Patrik S149: DM1 a; Stufe 31c-6)
 *
 * **Modale Verformungsenergie:** der Verlustfaktor einer Mode ist das Mittel der Verlustfaktoren
 * ihrer Werkstoffe, gewichtet mit dem Anteil der Verformungsenergie, den die Mode in jedem
 * Werkstoff hat (Johnson & Kienholz 1982 — aus Claudes Wissen, nicht nachgelesen):
 *
 *     η_Mode = Σ_m η_m · U_m / U,    U_m = ½ ∫_m κᵀ D κ + γᵀ S γ
 *
 * U_m wird an denselben Integrationspunkten mit denselben Groessen gerechnet wie die
 * Steifigkeitsmatrix (`Plattenelement::matrizen`), der Werkstoff je Punkt aus `Steife::werkstoff`.
 * Damit ist 2 Σ_m U_m = φᵀ K φ — fuer eine bezueglich M normierte Mode ω² (der Test prueft das).
 *
 * Einmal vorbereitet (je Element J⁻¹ und |det J|, je Integrationspunkt D, S und Werkstoff, die
 * Ansatzwerte im Bezugsdreieck), dann je Mode ausgewertet.
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek.
 */
#pragma once

#include "InstrumentBody/Platte.hpp"

#include <array>
#include <cmath>
#include <functional>
#include <vector>

namespace lumi::koerper
{

class Verformungsenergie
{
public:
    /// `werkstoffe` = Zahl der Werkstoffe (Plaetze 0 … werkstoffe − 1 aus `Steife::werkstoff`).
    Verformungsenergie(const PlattenSystem& s, const Plattenelement& element, const std::function<Steife(Punkt2)>& steife,
                       int werkstoffe)
        : m_werkstoffe(werkstoffe), m_nk(element.knoten())
    {
        const auto& punkte = element.integrationsPunkte();
        m_np = static_cast<int>(punkte.size());
        std::vector<double> n, dxi, deta;
        for (const auto& [xi, eta, gw] : punkte)
        {
            element.basis(xi, eta, n, dxi, deta);
            m_n.insert(m_n.end(), n.begin(), n.end());
            m_dxi.insert(m_dxi.end(), dxi.begin(), dxi.end());
            m_deta.insert(m_deta.end(), deta.begin(), deta.end());
            m_gewicht.push_back(gw);
        }
        m_elemente.reserve(s.elemente.size());
        m_punkte.reserve(s.elemente.size() * static_cast<std::size_t>(m_np));
        for (const auto& ek : s.elemente)
        {
            const Punkt2 e0 = s.knoten[ek[0]], e1 = s.knoten[ek[1]], e2 = s.knoten[ek[2]];
            const double j11 = e1.x - e0.x, j12 = e2.x - e0.x, j21 = e1.y - e0.y, j22 = e2.y - e0.y;
            const double det = j11 * j22 - j12 * j21;
            Element el;
            el.i = {j22 / det, -j12 / det, -j21 / det, j11 / det};  // J⁻¹: i11, i12, i21, i22
            el.betrag = std::abs(det);
            el.frei.reserve(static_cast<std::size_t>(3 * m_nk));
            for (int a = 0; a < m_nk; ++a)
                for (int g = 0; g < 3; ++g) el.frei.push_back(s.frei[static_cast<std::size_t>(3 * ek[static_cast<std::size_t>(a)] + g)]);
            m_elemente.push_back(std::move(el));
            for (const auto& [xi, eta, gw] : punkte)
            {
                const Steife st = steife({e0.x + j11 * xi + j12 * eta, e0.y + j21 * xi + j22 * eta});
                const double h3 = st.h * st.h * st.h / 12.0;
                Punkt p;
                for (int i = 0; i < 6; ++i) p.d[static_cast<std::size_t>(i)] = h3 * st.q[static_cast<std::size_t>(i)];
                for (int i = 0; i < 3; ++i) p.s[static_cast<std::size_t>(i)] = kSchubKorrektur * st.h * st.g[static_cast<std::size_t>(i)];
                p.werkstoff = st.werkstoff;
                m_punkte.push_back(p);
            }
        }
    }

    [[nodiscard]] int werkstoffe() const { return m_werkstoffe; }

    /// U_m je Werkstoff (J bei einer bezueglich M normierten Form: ½ ω² aufgeteilt); was keinem
    /// Werkstoff gehoert (Platz −1), zaehlt nicht mit. `form` nummeriert wie K und M.
    [[nodiscard]] std::vector<double> jeWerkstoff(const std::vector<double>& form) const
    {
        std::vector<double> u(static_cast<std::size_t>(m_werkstoffe), 0.0);
        std::vector<double> w(static_cast<std::size_t>(m_nk)), fx(static_cast<std::size_t>(m_nk)), fy(static_cast<std::size_t>(m_nk));
        for (std::size_t t = 0; t < m_elemente.size(); ++t)
        {
            const Element& el = m_elemente[t];
            for (int a = 0; a < m_nk; ++a)
            {
                const auto wert = [&](int g) {
                    const int i = el.frei[static_cast<std::size_t>(3 * a + g)];
                    return i >= 0 ? form[static_cast<std::size_t>(i)] : 0.0;
                };
                w[static_cast<std::size_t>(a)] = wert(0);
                fx[static_cast<std::size_t>(a)] = wert(1);
                fy[static_cast<std::size_t>(a)] = wert(2);
            }
            for (int g = 0; g < m_np; ++g)
            {
                const Punkt& p = m_punkte[t * static_cast<std::size_t>(m_np) + static_cast<std::size_t>(g)];
                if (p.werkstoff < 0 || p.werkstoff >= m_werkstoffe) continue;
                const std::size_t basis = static_cast<std::size_t>(g) * static_cast<std::size_t>(m_nk);
                double k1 = 0.0, k2 = 0.0, k6 = 0.0, gx = 0.0, gy = 0.0;
                for (int a = 0; a < m_nk; ++a)
                {
                    const std::size_t i = basis + static_cast<std::size_t>(a);
                    // ∂N/∂x = ∂N/∂ξ·∂ξ/∂x + ∂N/∂η·∂η/∂x — wie in `matrizen`
                    const double nx = m_dxi[i] * el.i[0] + m_deta[i] * el.i[2];
                    const double ny = m_dxi[i] * el.i[1] + m_deta[i] * el.i[3];
                    const std::size_t q = static_cast<std::size_t>(a);
                    k1 += nx * fx[q];
                    k2 += ny * fy[q];
                    k6 += ny * fx[q] + nx * fy[q];
                    gx += nx * w[q] - m_n[i] * fx[q];
                    gy += ny * w[q] - m_n[i] * fy[q];
                }
                const auto& d = p.d;
                const double biegung = k1 * (d[0] * k1 + d[1] * k2 + d[2] * k6) + k2 * (d[1] * k1 + d[3] * k2 + d[4] * k6) +
                                       k6 * (d[2] * k1 + d[4] * k2 + d[5] * k6);
                const double schub = gx * (p.s[0] * gx + p.s[1] * gy) + gy * (p.s[1] * gx + p.s[2] * gy);
                u[static_cast<std::size_t>(p.werkstoff)] += 0.5 * m_gewicht[static_cast<std::size_t>(g)] * el.betrag * (biegung + schub);
            }
        }
        return u;
    }

    /// Die Anteile U_m / U je Werkstoff (Summe 1); leer, wenn die Form keine Energie traegt.
    [[nodiscard]] std::vector<double> anteile(const std::vector<double>& form) const
    {
        std::vector<double> u = jeWerkstoff(form);
        double summe = 0.0;
        for (const double v : u) summe += v;
        if (!(summe > 0.0)) return {};
        for (double& v : u) v /= summe;
        return u;
    }

private:
    struct Element
    {
        std::array<double, 4> i{};  ///< J⁻¹: i11, i12, i21, i22
        double betrag = 0.0;        ///< |det J|
        std::vector<int> frei;      ///< je Knoten (w, φx, φy) der Platz in K und M; −1 = gehalten
    };
    struct Punkt
    {
        std::array<double, 6> d{};  ///< D = h³/12 · Q̄: 11, 12, 16, 22, 26, 66
        std::array<double, 3> s{};  ///< S = κs h Ḡ: xz, (xz,yz), yz
        int werkstoff = -1;
    };

    int m_werkstoffe = 0;
    int m_nk = 0;
    int m_np = 0;
    std::vector<double> m_n, m_dxi, m_deta;  ///< je Integrationspunkt nk Werte im Bezugsdreieck
    std::vector<double> m_gewicht;           ///< je Integrationspunkt
    std::vector<Element> m_elemente;
    std::vector<Punkt> m_punkte;             ///< je Element je Integrationspunkt
};

}  // namespace lumi::koerper
