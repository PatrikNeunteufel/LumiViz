/**
 * @file NetzCdt.hpp
 * @brief Der Netzgeber mit CDT (artem-ogre/CDT 2.0.1, MPL-2.0): Delaunay mit
 *        Zwangskanten und Ruppert-Verfeinerung (Entscheid Patrik S147: BM4 a)
 *
 * **Die einzige Datei, die CDT einbindet.** Wer sie einbindet, braucht das
 * External `CDT` (nur Unit-Tests und `InstrumentBodyTool`, nicht die App).
 *
 * Ablauf: alle Punkte (Umriss und Zwangslinien, doppelte zusammengelegt) und
 * Kanten einfuegen, sich kreuzende Zwangskanten aufloesen lassen; die Dreiecke
 * ausserhalb des Umrisses und in den Loechern (KQ7 a) sammeln; nach groesster Flaeche, dann nach kleinstem
 * Winkel verfeinern (zweimal, bis nichts mehr kommt); die aeusseren entfernen.
 * Knoten in der Reihenfolge, wie sie kommen (`AsProvided`) — zwei Laeufe geben
 * dasselbe Netz. Fehler von CDT kommen als Text in `Netz::fehler` zurueck.
 */
#pragma once

#include "InstrumentBody/Netz.hpp"

#include <CDT.h>

#include <algorithm>
#include <map>
#include <utility>

namespace lumi::koerper
{

class NetzgeberCdt final : public Netzgeber
{
public:
    [[nodiscard]] std::string name() const override { return "CDT 2.0.1"; }

    [[nodiscard]] Netz vernetze(const NetzAuftrag& auftrag) const override
    {
        Netz netz;
        if (auftrag.umriss.size() < 3 || auftrag.kante <= 0.0)
        {
            netz.fehler = "outline needs three points and the edge length must be positive";
            return netz;
        }
        try
        {
            // Punkte: gleiche Lage = ein Knoten
            std::vector<CDT::V2d<double>> punkte;
            std::map<std::pair<double, double>, CDT::VertInd> schon;
            const auto knoten = [&](Punkt2 p) {
                const auto [it, neu] = schon.try_emplace({p.x, p.y}, static_cast<CDT::VertInd>(punkte.size()));
                if (neu) punkte.emplace_back(p.x, p.y);
                return it->second;
            };
            std::vector<CDT::Edge> kanten;
            const auto zug = [&](const std::vector<Punkt2>& linie, bool geschlossen) {
                const std::size_t n = linie.size();
                for (std::size_t k = 0; k + (geschlossen ? 0 : 1) < n; ++k)
                {
                    const CDT::VertInd a = knoten(linie[k]), b = knoten(linie[(k + 1) % n]);
                    if (a != b) kanten.emplace_back(a, b);
                }
            };
            zug(auftrag.umriss, true);
            for (const auto& loch : auftrag.loecher)
            {
                if (loch.size() < 3)
                {
                    netz.fehler = "a hole needs three points";
                    return netz;
                }
                zug(loch, true);
            }
            for (const auto& linie : auftrag.zwangslinien) zug(linie, false);

            CDT::Triangulation<double> cdt(CDT::VertexInsertionOrder::AsProvided,
                                           CDT::IntersectingConstraintEdges::TryResolve, auftrag.kante * 1e-6);
            cdt.insertVertices(punkte);
            cdt.insertEdges(kanten);
            CDT::TriIndUSet aussen = cdt.collectOuterTriangles();
            // Loecher (KQ7 a): ihr Rand ist Zwangskante, ein Dreieck liegt also ganz drin oder ganz
            // draussen — der Schwerpunkt entscheidet. Sie zaehlen wie die aeusseren: nicht verfeinert,
            // am Ende entfernt.
            if (!auftrag.loecher.empty())
            {
                for (CDT::TriInd t = 0; t < static_cast<CDT::TriInd>(cdt.triangles.size()); ++t)
                {
                    if (aussen.count(t) > 0) continue;
                    const auto& v = cdt.triangles[t].vertices;
                    const Punkt2 mitte{(cdt.vertices[v[0]].x + cdt.vertices[v[1]].x + cdt.vertices[v[2]].x) / 3.0,
                                       (cdt.vertices[v[0]].y + cdt.vertices[v[1]].y + cdt.vertices[v[2]].y) / 3.0};
                    if (imLoch(auftrag.loecher, mitte)) aussen.insert(t);
                }
            }
            const auto budget = static_cast<CDT::VertInd>(20.0 * flaecheDesUmrisses(auftrag) / groessteFlaeche(auftrag.kante) + 10000);
            const double kurz = auftrag.kante * 1e-3;
            for (int runde = 0; runde < 2; ++runde)
            {
                cdt.refineTriangles(budget, CDT::RefinementCriterion::LargestArea, groessteFlaeche(auftrag.kante), &aussen, kurz);
                const CDT::Unrefined u = cdt.refineTriangles(budget, CDT::RefinementCriterion::SmallestAngle,
                                                             CDT::degToRad(auftrag.kleinsterWinkelGrad), &aussen, kurz);
                netz.unverfeinert = u.shortEdgeTriangles + u.circumcenterOutside + u.circumcenterOnVertex + u.sharpFixedCorner +
                                    u.shortEdges + u.splitVertexInvalid;
            }
            cdt.finalizeTriangulation(aussen);

            for (const auto& v : cdt.vertices) netz.knoten.push_back({v.x, v.y});
            for (const auto& t : cdt.triangles)
            {
                std::array<int, 3> d{static_cast<int>(t.vertices[0]), static_cast<int>(t.vertices[1]), static_cast<int>(t.vertices[2])};
                if (flaeche(netz, d) < 0.0) std::swap(d[1], d[2]);
                netz.dreiecke.push_back(d);
            }
            for (const auto& e : cdt.fixedEdges)
                netz.zwangskanten.push_back({static_cast<int>(std::min(e.v1(), e.v2())), static_cast<int>(std::max(e.v1(), e.v2()))});
            std::sort(netz.zwangskanten.begin(), netz.zwangskanten.end());
            netz.lochkanten = lochkantenVon(netz, auftrag.loecher, auftrag.kante * 1e-6);
        }
        catch (const std::exception& e)
        {
            netz = Netz{};
            netz.fehler = std::string("CDT: ") + e.what();
        }
        return netz;
    }

private:
    static double flaecheDesUmrisses(const NetzAuftrag& a) { return flaeche(a.umriss); }
};

}  // namespace lumi::koerper
