/**
 * @file Platte.hpp
 * @brief Die Platte des Instrumentkoerpers: Reissner-Mindlin, orthotrop, Dicke je
 *        Punkt; Steifigkeits- und Massenmatrix aus Dreiecken mit Lagrange-Ansaetzen
 *        frei waehlbarer Ordnung (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide
 *        Patrik S147: BM1 a, EL1 a; Stufe 31c-3)
 *
 * **Modell** wie Chabassier, Joly & Chaigne 2013 und MAESSTRO: eine heterogene
 * orthotrope Platte, Rippen und Stege als dickere Stellen mit ihrem eigenen
 * Material und ihrer Faser (RT-0425 §2.1: „each point belonging to a rib or not,
 * and to a bridge or not"). Groessen je Punkt: Auslenkung w und die Drehungen φx,
 * φy der Normalen. Kruemmung κ = (φx,x; φy,y; φx,y + φy,x), Querschub
 * γ = (w,x − φx; w,y − φy).
 *
 *     U = ½ ∫ κᵀ D κ + γᵀ S γ,   T = ½ ∫ ρh ẇ² + ρh³/12 (φ̇x² + φ̇y²)
 *
 * mit D = h³/12 · Q̄ (die ebene Steifigkeit, um die Faser gedreht) und
 * S = κs h Ḡ (Querschub, κs = 5/6).
 *
 * **Element (EL1 a)** hinter der kleinen Schnittstelle `Plattenelement`: die erste
 * Umsetzung `LagrangeDreieck` mit Ordnung k, volle Integration (Gauss auf dem
 * zusammengeklappten Quadrat, k + 2 Punkte je Richtung).
 *
 * **Auflager:** am Rand eingespannt (w = φx = φy = 0; der Boden im Flügel, BM1 a)
 * oder — fuer Pruefplatten mit achsparallelen Raendern — gelenkig (w = 0 und die
 * Drehung laengs des Randes = 0; die Navier-Loesung).
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek; K und M kommen als
 * `DuenneMatrix` (BM5-1 a).
 */
#pragma once

#include "InstrumentBody/Bauplan.hpp"
#include "InstrumentBody/DuenneMatrix.hpp"
#include "InstrumentBody/Netz.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <numbers>
#include <string>
#include <utility>
#include <vector>

namespace lumi::koerper
{

inline constexpr double kSchubKorrektur = 5.0 / 6.0;

// --- Werkstoff am Punkt -----------------------------------------------------------

/// Was die Platte an einem Punkt traegt.
struct Steife
{
    double h = 0.0;                 ///< Dicke gesamt, m
    double rho = 0.0;               ///< kg/m³
    std::array<double, 6> q{};      ///< Q̄ symmetrisch: 11, 12, 16, 22, 26, 66 (Pa)
    std::array<double, 3> g{};      ///< Ḡ symmetrisch: xz, (xz,yz), yz (Pa)
};

/// Die ebene Steifigkeit und der Querschub eines Werkstoffs, um `winkelGrad` gegen
/// die x-Achse gedreht (die Faser = Achse 1). Formeln nach R. M. Jones, *Mechanics of
/// Composite Materials* (Q̄ der Lamina).
[[nodiscard]] inline Steife gedreht(const Material& material, double winkelGrad)
{
    const Material m = vervollstaendigt(material);
    const double nu21 = m.nuxy * m.ey / m.ex;
    const double nenner = 1.0 - m.nuxy * nu21;
    const double q11 = m.ex / nenner, q22 = m.ey / nenner, q12 = m.nuxy * m.ey / nenner, q66 = m.gxy;
    const double w = winkelGrad * std::numbers::pi / 180.0;
    const double c = std::cos(w), s = std::sin(w);
    const double c2 = c * c, s2 = s * s, c4 = c2 * c2, s4 = s2 * s2, sc = s * c;
    Steife st;
    st.rho = m.dichte;
    st.q[0] = q11 * c4 + 2.0 * (q12 + 2.0 * q66) * s2 * c2 + q22 * s4;                     // 11
    st.q[1] = (q11 + q22 - 4.0 * q66) * s2 * c2 + q12 * (s4 + c4);                          // 12
    st.q[2] = (q11 - q12 - 2.0 * q66) * sc * c2 + (q12 - q22 + 2.0 * q66) * sc * s2;        // 16
    st.q[3] = q11 * s4 + 2.0 * (q12 + 2.0 * q66) * s2 * c2 + q22 * c4;                     // 22
    st.q[4] = (q11 - q12 - 2.0 * q66) * sc * s2 + (q12 - q22 + 2.0 * q66) * sc * c2;        // 26
    st.q[5] = (q11 + q22 - 2.0 * q12 - 2.0 * q66) * s2 * c2 + q66 * (s4 + c4);             // 66
    // Ḡ = R diag(G13, G23) Rᵀ, R die Drehung um w
    st.g[0] = m.gxz * c2 + m.gyz * s2;
    st.g[1] = (m.gxz - m.gyz) * sc;
    st.g[2] = m.gxz * s2 + m.gyz * c2;
    return st;
}

/// Wo ein Punkt auf einer Leiste liegt: Hoehe und Richtung dort; `drauf` = in ihrer Breite.
struct AufLeiste
{
    bool drauf = false;
    double hoehe = 0.0;
    double richtungGrad = 0.0;
};

[[nodiscard]] inline AufLeiste aufLeiste(const Leiste& l, Punkt2 p)
{
    AufLeiste best;
    double bestAbstand = std::numeric_limits<double>::infinity();
    for (std::size_t k = 0; k + 1 < l.linie.size(); ++k)
    {
        const Punkt2 a = l.linie[k], b = l.linie[k + 1];
        const double dx = b.x - a.x, dy = b.y - a.y, l2 = dx * dx + dy * dy;
        if (l2 <= 0.0) continue;
        const double t = std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / l2, 0.0, 1.0);
        const double d = std::hypot(p.x - (a.x + t * dx), p.y - (a.y + t * dy));
        if (d >= bestAbstand) continue;
        const double breite = l.breite[k] + t * (l.breite[k + 1] - l.breite[k]);
        bestAbstand = d;
        best.drauf = d <= breite / 2.0;
        best.hoehe = l.hoehe[k] + t * (l.hoehe[k + 1] - l.hoehe[k]);
        best.richtungGrad = std::atan2(dy, dx) * 180.0 / std::numbers::pi;
    }
    return best;
}

/// Was die Platte am Punkt `p` traegt: auf einem Steg dessen Material mit der Faser
/// laengs des Stegs, auf einer Rippe ebenso, sonst die Platte mit ihrer Faser; die
/// Dicke ist die der Platte plus die Hoehe der Leiste (der Steg geht vor).
[[nodiscard]] inline Steife steifeBei(const Bauplan& plan, Punkt2 p)
{
    const double platte = plan.platte.dickeBei(p);
    for (const auto* leisten : {&plan.stege, &plan.rippen})
    {
        for (const Leiste& l : *leisten)
        {
            const AufLeiste a = aufLeiste(l, p);
            if (!a.drauf) continue;
            const Material* m = findeMaterial(plan, l.material);
            if (!m) continue;
            Steife s = gedreht(*m, a.richtungGrad);
            s.h = platte + a.hoehe;
            return s;
        }
    }
    const Material* m = findeMaterial(plan, plan.platte.material);
    Steife s = m ? gedreht(*m, plan.platte.faserWinkelGrad) : Steife{};
    s.h = platte;
    return s;
}

// --- Integration -----------------------------------------------------------------

/// Gauss-Legendre auf [0, 1]: Stellen und Gewichte, n Punkte (Newton auf P_n).
[[nodiscard]] inline std::vector<std::pair<double, double>> gaussLegendre01(int n)
{
    std::vector<std::pair<double, double>> aus;
    for (int i = 1; i <= n; ++i)
    {
        double x = std::cos(std::numbers::pi * (i - 0.25) / (n + 0.5));
        double dp = 0.0;
        for (int it = 0; it < 100; ++it)
        {
            double p0 = 1.0, p1 = x;
            for (int k = 2; k <= n; ++k)
            {
                const double p2 = ((2.0 * k - 1.0) * x * p1 - (k - 1.0) * p0) / k;
                p0 = p1;
                p1 = p2;
            }
            if (n == 1) p0 = 1.0;
            dp = n * (x * p1 - p0) / (x * x - 1.0);
            const double dx = p1 / dp;
            x -= dx;
            if (std::abs(dx) < 1e-15) break;
        }
        aus.push_back({(1.0 + x) / 2.0, 1.0 / ((1.0 - x * x) * dp * dp)});  // Gewicht auf [0,1]: 2/(…)/2
    }
    return aus;
}

/// Punkte (ξ, η, Gewicht) auf dem Bezugsdreieck (0,0), (1,0), (0,1) — Gauss auf dem
/// zusammengeklappten Quadrat; genau fuer Polynome bis zum Grad 2n − 2.
[[nodiscard]] inline std::vector<std::array<double, 3>> dreiecksPunkte(int n)
{
    const auto g = gaussLegendre01(n);
    std::vector<std::array<double, 3>> aus;
    for (const auto& [u, wu] : g)
        for (const auto& [v, wv] : g) aus.push_back({u, v * (1.0 - u), wu * wv * (1.0 - u)});
    return aus;
}

// --- Elemente ----------------------------------------------------------------------

/// Die kleine Schnittstelle der Bauart eines Elements (EL1 a). Die Knoten eines
/// Elements in fester Ordnung: zuerst die drei Ecken, dann je Kante (0–1, 1–2, 2–0)
/// ihre inneren Knoten von der ersten zur zweiten Ecke, dann die im Inneren.
class Plattenelement
{
public:
    virtual ~Plattenelement() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual int ordnung() const = 0;
    /// Knoten je Kante ohne die Ecken, und im Inneren.
    [[nodiscard]] int knotenJeKante() const { return ordnung() - 1; }
    [[nodiscard]] int knotenInnen() const { return (ordnung() - 1) * (ordnung() - 2) / 2; }
    [[nodiscard]] int knoten() const { return 3 + 3 * knotenJeKante() + knotenInnen(); }
    /// Lage der Knoten in Bezugskoordinaten (ξ, η), in der Ordnung oben.
    [[nodiscard]] virtual std::vector<std::array<double, 2>> lage() const = 0;
    /// Element-Steifigkeit und -Masse (je (3·knoten())², zeilenweise; Groessen je Knoten
    /// w, φx, φy) eines Dreiecks mit diesen Ecken; `steife` liefert den Werkstoff an
    /// einem Punkt. Zurueck die Masse ∫ρh des Elements.
    virtual double matrizen(const std::array<Punkt2, 3>& ecken, const std::function<Steife(Punkt2)>& steife,
                            std::vector<double>& k, std::vector<double>& m) const = 0;
};

/// Lagrange-Dreieck der Ordnung k (1 … 6) fuer w, φx und φy (EL1 a).
class LagrangeDreieck final : public Plattenelement
{
public:
    explicit LagrangeDreieck(int ordnung) : m_k(std::clamp(ordnung, 1, 6))
    {
        // die Knoten als Mehrfachindex (a, b, c), a + b + c = k, in der Ordnung der Schnittstelle
        const int k = m_k;
        m_index = {{k, 0, 0}, {0, k, 0}, {0, 0, k}};
        for (int i = 1; i < k; ++i) m_index.push_back({k - i, i, 0});  // Kante 0–1
        for (int i = 1; i < k; ++i) m_index.push_back({0, k - i, i});  // Kante 1–2
        for (int i = 1; i < k; ++i) m_index.push_back({i, 0, k - i});  // Kante 2–0
        for (int b = 1; b < k; ++b)
            for (int c = 1; b + c < k; ++c) m_index.push_back({k - b - c, b, c});
        m_punkte = dreiecksPunkte(k + 2);
    }

    [[nodiscard]] std::string name() const override { return "Lagrange P" + std::to_string(m_k); }
    [[nodiscard]] int ordnung() const override { return m_k; }

    [[nodiscard]] std::vector<std::array<double, 2>> lage() const override
    {
        std::vector<std::array<double, 2>> aus;
        for (const auto& i : m_index) aus.push_back({static_cast<double>(i[1]) / m_k, static_cast<double>(i[2]) / m_k});
        return aus;
    }

    /// Die Ansatzfunktionen und ihre Ableitungen nach ξ und η an (ξ, η).
    void basis(double xi, double eta, std::vector<double>& n, std::vector<double>& dxi, std::vector<double>& deta) const
    {
        const double l[3] = {1.0 - xi - eta, xi, eta};
        const double dl[3][2] = {{-1.0, -1.0}, {1.0, 0.0}, {0.0, 1.0}};
        const std::size_t z = m_index.size();
        n.assign(z, 0.0);
        dxi.assign(z, 0.0);
        deta.assign(z, 0.0);
        for (std::size_t i = 0; i < z; ++i)
        {
            double f[3], df[3];
            for (int r = 0; r < 3; ++r) faktor(m_index[i][r], l[r], f[r], df[r]);
            n[i] = f[0] * f[1] * f[2];
            for (int r = 0; r < 3; ++r)
            {
                const double rest = f[(r + 1) % 3] * f[(r + 2) % 3];
                dxi[i] += df[r] * dl[r][0] * rest;
                deta[i] += df[r] * dl[r][1] * rest;
            }
        }
    }

    double matrizen(const std::array<Punkt2, 3>& e, const std::function<Steife(Punkt2)>& steife, std::vector<double>& k,
                    std::vector<double>& m) const override
    {
        const int nk = knoten(), nd = 3 * nk;
        k.assign(static_cast<std::size_t>(nd) * nd, 0.0);
        m.assign(static_cast<std::size_t>(nd) * nd, 0.0);
        // affin: x = e0 + J (ξ, η)
        const double j11 = e[1].x - e[0].x, j12 = e[2].x - e[0].x, j21 = e[1].y - e[0].y, j22 = e[2].y - e[0].y;
        const double det = j11 * j22 - j12 * j21;
        const double i11 = j22 / det, i12 = -j12 / det, i21 = -j21 / det, i22 = j11 / det;  // J⁻¹
        std::vector<double> n, dxi, deta, nx(static_cast<std::size_t>(nk)), ny(static_cast<std::size_t>(nk));
        double masse = 0.0;
        for (const auto& [xi, eta, gw] : m_punkte)
        {
            basis(xi, eta, n, dxi, deta);
            for (int i = 0; i < nk; ++i)
            {
                nx[i] = dxi[i] * i11 + deta[i] * i21;  // ∂N/∂x = ∂N/∂ξ·∂ξ/∂x + ∂N/∂η·∂η/∂x
                ny[i] = dxi[i] * i12 + deta[i] * i22;
            }
            const Punkt2 p{e[0].x + j11 * xi + j12 * eta, e[0].y + j21 * xi + j22 * eta};
            const Steife s = steife(p);
            const double w = gw * std::abs(det);
            const double h3 = s.h * s.h * s.h / 12.0;
            const double d11 = h3 * s.q[0], d12 = h3 * s.q[1], d16 = h3 * s.q[2], d22 = h3 * s.q[3], d26 = h3 * s.q[4], d66 = h3 * s.q[5];
            const double s11 = kSchubKorrektur * s.h * s.g[0], s12 = kSchubKorrektur * s.h * s.g[1], s22 = kSchubKorrektur * s.h * s.g[2];
            const double mw = s.rho * s.h, mr = s.rho * h3;
            masse += mw * w;
            for (int a = 0; a < nk; ++a)
            {
                // B-Spalten des Knotens a: Kruemmung (κ1, κ2, κ6) und Schub (γx, γy) je Groesse (w, φx, φy)
                const double ka[3][3] = {{0.0, nx[a], 0.0}, {0.0, 0.0, ny[a]}, {0.0, ny[a], nx[a]}};
                const double ga[2][3] = {{nx[a], -n[a], 0.0}, {ny[a], 0.0, -n[a]}};
                for (int b = 0; b < nk; ++b)
                {
                    const double kb[3][3] = {{0.0, nx[b], 0.0}, {0.0, 0.0, ny[b]}, {0.0, ny[b], nx[b]}};
                    const double gb[2][3] = {{nx[b], -n[b], 0.0}, {ny[b], 0.0, -n[b]}};
                    for (int u = 0; u < 3; ++u)
                    {
                        // D·κ_b(v) und S·γ_b(v) zeilenweise
                        for (int v = 0; v < 3; ++v)
                        {
                            const double db0 = d11 * kb[0][v] + d12 * kb[1][v] + d16 * kb[2][v];
                            const double db1 = d12 * kb[0][v] + d22 * kb[1][v] + d26 * kb[2][v];
                            const double db2 = d16 * kb[0][v] + d26 * kb[1][v] + d66 * kb[2][v];
                            const double sb0 = s11 * gb[0][v] + s12 * gb[1][v];
                            const double sb1 = s12 * gb[0][v] + s22 * gb[1][v];
                            const double wert = ka[0][u] * db0 + ka[1][u] * db1 + ka[2][u] * db2 + ga[0][u] * sb0 + ga[1][u] * sb1;
                            k[static_cast<std::size_t>(3 * a + u) * nd + 3 * b + v] += w * wert;
                        }
                    }
                    const double nn = n[a] * n[b] * w;
                    m[static_cast<std::size_t>(3 * a) * nd + 3 * b] += mw * nn;
                    m[static_cast<std::size_t>(3 * a + 1) * nd + 3 * b + 1] += mr * nn;
                    m[static_cast<std::size_t>(3 * a + 2) * nd + 3 * b + 2] += mr * nn;
                }
            }
        }
        return masse;
    }

private:
    /// f(L) = Π_{m<a} (kL − m)/(m + 1) und f'(L).
    void faktor(int a, double l, double& f, double& df) const
    {
        f = 1.0;
        df = 0.0;
        for (int mm = 0; mm < a; ++mm)
        {
            const double g = (m_k * l - mm) / (mm + 1.0);
            const double dg = m_k / (mm + 1.0);
            df = df * g + f * dg;
            f *= g;
        }
    }

    int m_k;
    std::vector<std::array<int, 3>> m_index;
    std::vector<std::array<double, 3>> m_punkte;
};

// --- das System ---------------------------------------------------------------------

enum class Auflager : int
{
    Eingespannt = 0,       ///< w = φx = φy = 0 am ganzen Rand
    GelenkigAchsparallel,  ///< w = 0, Drehung laengs des Randes = 0; nur achsparallele Raender
};

/// Steifigkeit und Masse der Platte, nur die freien Groessen.
struct PlattenSystem
{
    std::string element;              ///< Name der Bauart
    int ordnung = 0;
    std::vector<Punkt2> knoten;       ///< alle Knoten des Systems (Ecken, Kanten, Inneres)
    std::vector<int> frei;            ///< je Groesse (3 je Knoten: w, φx, φy) ihr Platz in K und M; −1 = gehalten
    DuenneMatrix k, m;
    double masse = 0.0;               ///< ∫ ρh ueber die ganze Platte, kg
    std::string fehler;
};

/// Baut K und M einer Platte auf einem Netz. `steife` liefert den Werkstoff je Punkt.
[[nodiscard]] inline PlattenSystem baueSystem(const Netz& netz, const Plattenelement& element,
                                              const std::function<Steife(Punkt2)>& steife, Auflager auflager)
{
    PlattenSystem sys;
    sys.element = element.name();
    sys.ordnung = element.ordnung();
    if (netz.dreiecke.empty())
    {
        sys.fehler = "no triangles";
        return sys;
    }
    const int k = element.ordnung(), proKante = element.knotenJeKante(), innen = element.knotenInnen(), nk = element.knoten();
    const int nv = static_cast<int>(netz.knoten.size());

    // Kanten zaehlen: einmal = Rand
    std::map<std::pair<int, int>, int> kanten;  // (kleiner, groesser) → Zahl der Dreiecke
    for (const auto& d : netz.dreiecke)
        for (int r = 0; r < 3; ++r) ++kanten[{std::min(d[r], d[(r + 1) % 3]), std::max(d[r], d[(r + 1) % 3])}];
    std::map<std::pair<int, int>, int> kantenBasis;
    int naechster = nv;
    for (auto& [kante, zahl] : kanten)
    {
        kantenBasis[kante] = naechster;
        naechster += proKante;
    }
    const int innenBasis = naechster;
    const int knotenGesamt = innenBasis + innen * static_cast<int>(netz.dreiecke.size());

    // je Element die Knoten des Systems und ihre Lage
    sys.knoten.assign(static_cast<std::size_t>(knotenGesamt), {});
    for (int v = 0; v < nv; ++v) sys.knoten[v] = netz.knoten[v];
    const auto lage = element.lage();
    std::vector<std::vector<int>> elementKnoten(netz.dreiecke.size());
    std::vector<char> amRand(static_cast<std::size_t>(knotenGesamt), 0);
    std::vector<char> randWaagrecht(static_cast<std::size_t>(knotenGesamt), 0), randSenkrecht(static_cast<std::size_t>(knotenGesamt), 0);
    for (std::size_t t = 0; t < netz.dreiecke.size(); ++t)
    {
        const auto& d = netz.dreiecke[t];
        auto& ek = elementKnoten[t];
        ek.assign(d.begin(), d.end());
        for (int r = 0; r < 3; ++r)
        {
            const int a = d[r], b = d[(r + 1) % 3];
            const auto schluessel = std::make_pair(std::min(a, b), std::max(a, b));
            const int basis = kantenBasis[schluessel];
            for (int i = 1; i < k; ++i)
                ek.push_back(basis + (a < b ? i - 1 : k - i - 1));  // vom kleineren Knoten aus gezaehlt
        }
        for (int i = 0; i < innen; ++i) ek.push_back(innenBasis + static_cast<int>(t) * innen + i);
        const Punkt2 e0 = netz.knoten[d[0]], e1 = netz.knoten[d[1]], e2 = netz.knoten[d[2]];
        for (int i = 0; i < nk; ++i)
        {
            const double xi = lage[i][0], eta = lage[i][1];
            sys.knoten[ek[i]] = {e0.x + (e1.x - e0.x) * xi + (e2.x - e0.x) * eta, e0.y + (e1.y - e0.y) * xi + (e2.y - e0.y) * eta};
        }
        // Randknoten: die Ecken und Kantenknoten einer Kante, die nur ein Dreieck hat
        for (int r = 0; r < 3; ++r)
        {
            const int a = d[r], b = d[(r + 1) % 3];
            if (kanten[{std::min(a, b), std::max(a, b)}] != 1) continue;
            const Punkt2 pa = netz.knoten[a], pb = netz.knoten[b];
            const bool waagrecht = std::abs(pa.y - pb.y) <= 1e-12 * (1.0 + std::abs(pa.x - pb.x));
            const bool senkrecht = std::abs(pa.x - pb.x) <= 1e-12 * (1.0 + std::abs(pa.y - pb.y));
            if (auflager == Auflager::GelenkigAchsparallel && !waagrecht && !senkrecht)
            {
                sys.fehler = "simply supported needs axis-parallel edges";
                return sys;
            }
            std::vector<int> aufKante = {a, b};
            for (int i = 0; i < proKante; ++i) aufKante.push_back(ek[3 + r * proKante + i]);
            for (const int q : aufKante)
            {
                amRand[q] = 1;
                if (waagrecht) randWaagrecht[q] = 1;
                if (senkrecht) randSenkrecht[q] = 1;
            }
        }
    }

    // freie Groessen
    sys.frei.assign(static_cast<std::size_t>(3 * knotenGesamt), -1);
    int nFrei = 0;
    for (int q = 0; q < knotenGesamt; ++q)
    {
        for (int g = 0; g < 3; ++g)
        {
            bool gehalten = false;
            if (amRand[q])
            {
                if (auflager == Auflager::Eingespannt) gehalten = true;
                else if (g == 0) gehalten = true;                       // w
                else if (g == 1 && randWaagrecht[q]) gehalten = true;   // φx laengs eines waagrechten Randes
                else if (g == 2 && randSenkrecht[q]) gehalten = true;   // φy laengs eines senkrechten Randes
            }
            if (!gehalten) sys.frei[static_cast<std::size_t>(3 * q + g)] = nFrei++;
        }
    }

    // Muster
    {
        std::vector<std::vector<int>> zeilen(static_cast<std::size_t>(nFrei));
        for (const auto& ek : elementKnoten)
        {
            std::vector<int> f;
            for (const int q : ek)
                for (int g = 0; g < 3; ++g)
                    if (const int i = sys.frei[static_cast<std::size_t>(3 * q + g)]; i >= 0) f.push_back(i);
            for (const int i : f)
                for (const int j : f)
                    if (j >= i) zeilen[static_cast<std::size_t>(i)].push_back(j);
        }
        std::vector<std::vector<int>> kopie = zeilen;
        sys.k.setzeMuster(nFrei, zeilen);
        sys.m.setzeMuster(nFrei, kopie);
    }

    // Werte
    std::vector<double> ke, me;
    for (std::size_t t = 0; t < netz.dreiecke.size(); ++t)
    {
        const auto& d = netz.dreiecke[t];
        sys.masse += element.matrizen({netz.knoten[d[0]], netz.knoten[d[1]], netz.knoten[d[2]]}, steife, ke, me);
        const auto& ek = elementKnoten[t];
        const int nd = 3 * nk;
        for (int a = 0; a < nd; ++a)
        {
            const int i = sys.frei[static_cast<std::size_t>(3 * ek[a / 3] + a % 3)];
            if (i < 0) continue;
            for (int b = 0; b < nd; ++b)
            {
                const int j = sys.frei[static_cast<std::size_t>(3 * ek[b / 3] + b % 3)];
                if (j < i) continue;
                sys.k.addiere(i, j, ke[static_cast<std::size_t>(a) * nd + b]);
                sys.m.addiere(i, j, me[static_cast<std::size_t>(a) * nd + b]);
            }
        }
    }
    return sys;
}

}  // namespace lumi::koerper
