/**
 * @file Netz.hpp
 * @brief Das Netz des Instrumentkoerpers: die Schnittstelle des Netzgebers, die
 *        Pruefgroessen eines Netzes und was aus einem Bauplan vernetzt wird
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheid Patrik S147: BM4 a, Stufe 31c-2)
 *
 * **Schnittstelle (BM4 a, Muster BM5-1 a):** ein `Netzgeber` bekommt Umriss,
 * Zwangslinien und Elementgroesse und gibt Knoten und Dreiecke zurueck. Kein Typ
 * einer fremden Bibliothek geht ueber sie hinaus — die erste Umsetzung mit CDT
 * steht allein in `NetzCdt.hpp`; ein eigener Generator wird eine zweite
 * Umsetzung derselben Schnittstelle und mit denselben Pruefungen verglichen.
 *
 * **Zwangslinien** sind die Raender jeder Rippe und jedes Stegs (Mittellinie ±
 * halbe Breite): dort springt die Dicke, also sollen dort Kanten liegen. Sie
 * werden am Umriss abgeschnitten.
 *
 * **Elementgroesse** nach MAESSTRO (Elie, Cotte & Boutillon 2022, Gl. 1): ein
 * Viertel der kuerzesten Biegewelle quer zur Faser bei der hoechsten Frequenz,
 * λ = √(2π / f) · (D / (ρ h))^¼ mit D = E_y h³ / 12 — hier an der duennsten
 * Stelle der Platte.
 *
 * Reines C++ ohne Qt und ohne fremde Bibliothek.
 */
#pragma once

#include "InstrumentBody/Bauplan.hpp"

#include <array>
#include <cmath>
#include <numbers>
#include <string>
#include <vector>

namespace lumi::koerper
{

/// Was ein Netzgeber vernetzen soll.
struct NetzAuftrag
{
    std::vector<Punkt2> umriss;                    ///< geschlossen, der erste Punkt nicht wiederholt
    std::vector<std::vector<Punkt2>> zwangslinien;  ///< offene Linienzuege im Umriss; sie duerfen sich kreuzen
    double kante = 0.0;                            ///< m: Kantenlaenge der Elemente (gleichseitig gedacht)
    double kleinsterWinkelGrad = 25.0;             ///< kein Dreieck spitzer — ausser an Ecken der Eingabe
};

/// Ein Netz aus Dreiecken. Die Knoten jedes Dreiecks gegen den Uhrzeiger.
struct Netz
{
    std::vector<Punkt2> knoten;
    std::vector<std::array<int, 3>> dreiecke;
    /// Kanten auf dem Umriss oder einer Zwangslinie (Knotenpaare, kleinerer zuerst, sortiert)
    std::vector<std::array<int, 2>> zwangskanten;
    std::string fehler;  ///< leer = in Ordnung
    /// Was der Netzgeber nicht verfeinern konnte (Dreiecke oder Kanten); nur zur Auskunft
    std::size_t unverfeinert = 0;

    [[nodiscard]] bool operator==(const Netz&) const = default;
};

/// Die Schnittstelle (BM4 a). Eine Umsetzung ist zustandslos: zwei Aufrufe mit
/// demselben Auftrag geben dasselbe Netz.
class Netzgeber
{
public:
    virtual ~Netzgeber() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual Netz vernetze(const NetzAuftrag& auftrag) const = 0;
};

// --- Pruefgroessen ---------------------------------------------------------------

/// Die Flaeche eines Dreiecks, mit Vorzeichen (positiv = gegen den Uhrzeiger).
[[nodiscard]] inline double flaeche(const Netz& n, const std::array<int, 3>& d)
{
    const Punkt2 a = n.knoten[d[0]], b = n.knoten[d[1]], c = n.knoten[d[2]];
    return ((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y)) / 2.0;
}

/// Die Summe der Dreiecksflaechen.
[[nodiscard]] inline double flaeche(const Netz& n)
{
    double s = 0.0;
    for (const auto& d : n.dreiecke) s += flaeche(n, d);
    return s;
}

/// Der kleinste Innenwinkel eines Dreiecks in Grad.
[[nodiscard]] inline double kleinsterWinkelGrad(const Netz& n, const std::array<int, 3>& d)
{
    double kleinster = 180.0;
    for (int k = 0; k < 3; ++k)
    {
        const Punkt2 p = n.knoten[d[k]], a = n.knoten[d[(k + 1) % 3]], b = n.knoten[d[(k + 2) % 3]];
        const double ux = a.x - p.x, uy = a.y - p.y, vx = b.x - p.x, vy = b.y - p.y;
        const double w = std::atan2(std::abs(ux * vy - uy * vx), ux * vx + uy * vy);
        kleinster = std::min(kleinster, w * 180.0 / std::numbers::pi);
    }
    return kleinster;
}

/// Die laengste Kante eines Dreiecks in m.
[[nodiscard]] inline double laengsteKante(const Netz& n, const std::array<int, 3>& d)
{
    double l = 0.0;
    for (int k = 0; k < 3; ++k)
    {
        const Punkt2 a = n.knoten[d[k]], b = n.knoten[d[(k + 1) % 3]];
        l = std::max(l, std::hypot(b.x - a.x, b.y - a.y));
    }
    return l;
}

/// Die groesste Flaeche, die ein Dreieck bei der Kante `kante` haben soll: die eines
/// gleichseitigen Dreiecks dieser Kante.
[[nodiscard]] inline double groessteFlaeche(double kante)
{
    return std::sqrt(3.0) / 4.0 * kante * kante;
}

/// Liegt die Strecke a–b ganz auf Zwangskanten des Netzes? (Die Kanten duerfen sie in
/// Stuecke teilen; geprueft wird, dass ihre Laengen auf der Strecke die Strecke fuellen.)
[[nodiscard]] inline bool aufZwangskanten(const Netz& n, Punkt2 a, Punkt2 b, double toleranz = 1e-9)
{
    const double lx = b.x - a.x, ly = b.y - a.y;
    const double laenge = std::hypot(lx, ly);
    if (laenge <= 0.0) return true;
    double gedeckt = 0.0;
    for (const auto& k : n.zwangskanten)
    {
        const Punkt2 p = n.knoten[k[0]], q = n.knoten[k[1]];
        const auto abstand = [&](Punkt2 r) { return std::abs((r.x - a.x) * ly - (r.y - a.y) * lx) / laenge; };
        if (abstand(p) > toleranz * (1.0 + laenge) || abstand(q) > toleranz * (1.0 + laenge)) continue;
        const double tp = ((p.x - a.x) * lx + (p.y - a.y) * ly) / (laenge * laenge);
        const double tq = ((q.x - a.x) * lx + (q.y - a.y) * ly) / (laenge * laenge);
        const double lo = std::clamp(std::min(tp, tq), 0.0, 1.0), hi = std::clamp(std::max(tp, tq), 0.0, 1.0);
        gedeckt += (hi - lo) * laenge;
    }
    return std::abs(gedeckt - laenge) <= 1e-6 * (1.0 + laenge);
}

// --- was aus dem Bauplan vernetzt wird ------------------------------------------

/// Die kuerzeste Biegewelle in m einer orthotropen Platte der Dicke `h` bei `f` Hz,
/// quer zur Faser (MAESSTRO Gl. 1).
[[nodiscard]] inline double kuerzesteBiegewelle(const Material& m, double h, double f)
{
    const Material v = vervollstaendigt(m);
    const double d = v.ey * h * h * h / 12.0;
    return std::sqrt(2.0 * std::numbers::pi / f) * std::pow(d / (v.dichte * h), 0.25);
}

/// Die Kantenlaenge fuer Moden bis `fmax` Hz: ein Viertel der kuerzesten Biegewelle
/// an der duennsten Stelle der Platte (MAESSTRO: l_c ≤ λ_min / 4). 0 = kein Material.
[[nodiscard]] inline double elementKante(const Bauplan& plan, double fmax)
{
    const Material* m = findeMaterial(plan, plan.platte.material);
    if (!m || fmax <= 0.0) return 0.0;
    double h = plan.platte.dicke;
    for (const double v : plan.platte.dickeRaster.werte)
        if (!std::isnan(v)) h = std::min(h, v);
    return kuerzesteBiegewelle(*m, h, fmax) / 4.0;
}

/// Ein Linienzug um `abstand` m nach links (positiv) oder rechts verschoben; je Punkt
/// die Normale aus den anliegenden Stuecken gemittelt.
[[nodiscard]] inline std::vector<Punkt2> versetzt(const std::vector<Punkt2>& linie, const std::vector<double>& abstand)
{
    std::vector<Punkt2> aus;
    const std::size_t n = linie.size();
    for (std::size_t k = 0; k < n; ++k)
    {
        double nx = 0.0, ny = 0.0;
        for (const std::size_t s : {k == 0 ? k : k - 1, k + 1 < n ? k : k - 1})
        {
            const double dx = linie[s + 1].x - linie[s].x, dy = linie[s + 1].y - linie[s].y;
            const double l = std::hypot(dx, dy);
            if (l > 0.0)
            {
                nx += -dy / l;
                ny += dx / l;
            }
        }
        const double l = std::hypot(nx, ny);
        aus.push_back(l > 0.0 ? Punkt2{linie[k].x + abstand[k] * nx / l, linie[k].y + abstand[k] * ny / l} : linie[k]);
    }
    return aus;
}

/// Ein Linienzug am Umriss abgeschnitten: zurueck die Stuecke, die innen liegen.
[[nodiscard]] inline std::vector<std::vector<Punkt2>> imUmriss(const std::vector<Punkt2>& linie, const std::vector<Punkt2>& umriss)
{
    std::vector<std::vector<Punkt2>> stuecke;
    std::vector<Punkt2> aktuell;
    const auto schnitt = [&](Punkt2 a, Punkt2 b) {
        // der erste Schnittpunkt von a nach b mit dem Umriss
        double tBest = 2.0;
        for (std::size_t k = 0; k < umriss.size(); ++k)
        {
            const Punkt2 p = umriss[k], q = umriss[(k + 1) % umriss.size()];
            const double rx = b.x - a.x, ry = b.y - a.y, sx = q.x - p.x, sy = q.y - p.y;
            const double nenner = rx * sy - ry * sx;
            if (std::abs(nenner) < 1e-15) continue;
            const double t = ((p.x - a.x) * sy - (p.y - a.y) * sx) / nenner;
            const double u = ((p.x - a.x) * ry - (p.y - a.y) * rx) / nenner;
            if (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0) tBest = std::min(tBest, t);
        }
        return Punkt2{a.x + tBest * (b.x - a.x), a.y + tBest * (b.y - a.y)};
    };
    for (std::size_t k = 0; k < linie.size(); ++k)
    {
        const bool drin = innen(umriss, linie[k]);
        if (k > 0)
        {
            const bool vorherDrin = innen(umriss, linie[k - 1]);
            if (vorherDrin && !drin) aktuell.push_back(schnitt(linie[k - 1], linie[k]));
            if (!vorherDrin && drin) aktuell.push_back(schnitt(linie[k], linie[k - 1]));
        }
        if (drin) aktuell.push_back(linie[k]);
        else if (!aktuell.empty())
        {
            if (aktuell.size() >= 2) stuecke.push_back(aktuell);
            aktuell.clear();
        }
    }
    if (aktuell.size() >= 2) stuecke.push_back(aktuell);
    return stuecke;
}

/// Der Auftrag fuer einen Bauplan: der Umriss der Platte und die beiden Raender jeder
/// Rippe und jedes Stegs, am Umriss abgeschnitten; Kanten fuer Moden bis `fmax` Hz.
[[nodiscard]] inline NetzAuftrag netzAuftrag(const Bauplan& plan, double fmax)
{
    NetzAuftrag a;
    a.umriss = plan.platte.umriss;
    a.kante = elementKante(plan, fmax);
    for (const auto* leisten : {&plan.rippen, &plan.stege})
    {
        for (const Leiste& l : *leisten)
        {
            for (const double seite : {0.5, -0.5})
            {
                std::vector<double> abstand;
                for (const double b : l.breite) abstand.push_back(seite * b);
                for (auto& stueck : imUmriss(versetzt(l.linie, abstand), plan.platte.umriss)) a.zwangslinien.push_back(std::move(stueck));
            }
        }
    }
    return a;
}

}  // namespace lumi::koerper
