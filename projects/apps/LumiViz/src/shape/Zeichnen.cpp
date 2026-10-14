/**
 * @file Zeichnen.cpp
 * @brief Klang aus einer Form: Pen und Move point am Pfad (Stufe 20a)
 */

#include "shape/Zeichnen.hpp"

#include <QLineF>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace lumi::shape
{

namespace
{

/// Naeher als das liegen zwei Punkte aufeinander.
constexpr double kGleich = 1e-9;

bool gleich(QPointF a, QPointF b)
{
    return std::abs(a.x() - b.x()) < kGleich && std::abs(a.y() - b.y()) < kGleich;
}

/// Wo ein Ankerpunkt im Pfad steht: das Element seines Orts, und ob er das
/// Ende einer Kurve ist (dann steht ihr zweiter Griff gleich davor).
struct Anker
{
    int element = 0;
    bool kurvenEnde = false;
};

std::vector<Anker> alleAnker(const QPainterPath& pfad)
{
    std::vector<Anker> anker;
    for (int i = 0; i < pfad.elementCount(); ++i)
    {
        const QPainterPath::Element e = pfad.elementAt(i);
        if (e.type == QPainterPath::CurveToElement && i + 2 < pfad.elementCount())
        {
            anker.push_back({i + 2, true});
            i += 2;
            continue;
        }
        if (e.type == QPainterPath::MoveToElement || e.type == QPainterPath::LineToElement) anker.push_back({i, false});
    }
    return anker;
}

QPointF ort(const QPainterPath& pfad, int element)
{
    const QPainterPath::Element e = pfad.elementAt(element);
    return {e.x, e.y};
}

/// Ist der Zug geschlossen — liegt der letzte Ankerpunkt auf dem ersten?
bool geschlossen(const QPainterPath& pfad, const std::vector<Anker>& anker)
{
    return anker.size() > 2 && gleich(ort(pfad, anker.front().element), ort(pfad, anker.back().element));
}

/// Ein Element um `schritt` verschieben.
void verschiebe(QPainterPath& pfad, int element, QPointF schritt)
{
    const QPointF alt = ort(pfad, element);
    pfad.setElementPositionAt(element, alt.x() + schritt.x(), alt.y() + schritt.y());
}

/// Douglas-Peucker: welche Punkte zwischen `von` und `bis` bleiben muessen.
void vereinfache(const std::vector<QPointF>& punkte, std::size_t von, std::size_t bis, double toleranz,
                 std::vector<bool>& bleibt)
{
    if (bis <= von + 1) return;
    const QPointF a = punkte[von];
    const QPointF b = punkte[bis];
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    const double laenge = std::hypot(dx, dy);
    double weitest = -1.0;
    std::size_t wo = von;
    for (std::size_t i = von + 1; i < bis; ++i)
    {
        const QPointF p = punkte[i];
        // Abstand zur Geraden durch a und b; liegen a und b aufeinander (ein Zug zurueck zum Anfang), zu a
        const double abstand = laenge > kGleich ? std::abs(dy * (p.x() - a.x()) - dx * (p.y() - a.y())) / laenge
                                                : std::hypot(p.x() - a.x(), p.y() - a.y());
        if (abstand > weitest)
        {
            weitest = abstand;
            wo = i;
        }
    }
    if (weitest <= toleranz) return;
    bleibt[wo] = true;
    vereinfache(punkte, von, wo, toleranz, bleibt);
    vereinfache(punkte, wo, bis, toleranz, bleibt);
}

}  // namespace

int haengeFreihandAn(QPainterPath& pfad, const std::vector<QPointF>& zug, double toleranz)
{
    // doppelte Punkte (die Maus stand) heraus
    std::vector<QPointF> punkte;
    punkte.reserve(zug.size());
    for (const QPointF& p : zug)
        if (punkte.empty() || !gleich(punkte.back(), p)) punkte.push_back(p);
    if (punkte.empty()) return 0;

    int dazu = 0;
    if (pfad.elementCount() == 0)
    {
        pfad.moveTo(punkte.front());
        ++dazu;
    }
    else if (!gleich(pfad.currentPosition(), punkte.front()))
    {
        pfad.lineTo(punkte.front());
        ++dazu;
    }
    if (punkte.size() < 2) return dazu;

    // glaetten: gleitendes Mittel ueber fuenf Punkte, die Enden bleiben, wo sie sind
    std::vector<QPointF> glatt = punkte;
    constexpr std::size_t kBreite = 2;
    for (std::size_t i = 1; i + 1 < punkte.size(); ++i)
    {
        const std::size_t von = i >= kBreite ? i - kBreite : 0;
        const std::size_t bis = std::min(punkte.size() - 1, i + kBreite);
        QPointF summe;
        for (std::size_t j = von; j <= bis; ++j) summe += punkte[j];
        glatt[i] = summe / static_cast<double>(bis - von + 1);
    }

    // vereinfachen: nur die Punkte, ohne die der Zug weiter als `toleranz` abwiche
    std::vector<bool> bleibt(glatt.size(), false);
    bleibt.front() = true;
    bleibt.back() = true;
    vereinfache(glatt, 0, glatt.size() - 1, std::max(toleranz, 0.0), bleibt);
    std::vector<QPointF> anker;
    for (std::size_t i = 0; i < glatt.size(); ++i)
        if (bleibt[i]) anker.push_back(glatt[i]);

    // Kurven durch die Ankerpunkte (Catmull-Rom als kubische Bezier-Stuecke);
    // an den Enden zeigt die Tangente zum Nachbarn
    for (std::size_t i = 0; i + 1 < anker.size(); ++i)
    {
        const QPointF p0 = anker[i == 0 ? 0 : i - 1];
        const QPointF p1 = anker[i];
        const QPointF p2 = anker[i + 1];
        const QPointF p3 = anker[std::min(i + 2, anker.size() - 1)];
        pfad.cubicTo(p1 + (p2 - p0) / 6.0, p2 - (p3 - p1) / 6.0, p2);
        ++dazu;
    }
    return dazu;
}

bool haengeEckeAn(QPainterPath& pfad, QPointF punkt)
{
    if (pfad.elementCount() == 0)
    {
        pfad.moveTo(punkt);
        return true;
    }
    if (gleich(pfad.currentPosition(), punkt)) return false;
    pfad.lineTo(punkt);
    return true;
}

std::vector<QPointF> ankerpunkte(const QPainterPath& pfad)
{
    std::vector<Anker> anker = alleAnker(pfad);
    if (geschlossen(pfad, anker)) anker.pop_back();
    std::vector<QPointF> punkte;
    punkte.reserve(anker.size());
    for (const Anker& a : anker) punkte.push_back(ort(pfad, a.element));
    return punkte;
}

bool laesstSichSchliessen(const QPainterPath& pfad)
{
    const std::vector<QPointF> punkte = ankerpunkte(pfad);
    if (punkte.size() >= 3) return true;
    if (punkte.size() < 2) return false;
    // zwei Punkte mit einer Kurve dazwischen umschliessen eine Flaeche; zwei mit einer Strecke nicht
    for (int i = 0; i < pfad.elementCount(); ++i)
        if (pfad.elementAt(i).type == QPainterPath::CurveToElement) return true;
    return false;
}

int naechsterAnker(const QPainterPath& pfad, QPointF punkt, double radius)
{
    const std::vector<QPointF> punkte = ankerpunkte(pfad);
    int bester = -1;
    double naechst = radius;
    for (std::size_t i = 0; i < punkte.size(); ++i)
    {
        const double abstand = QLineF(punkte[i], punkt).length();
        if (abstand <= naechst)
        {
            naechst = abstand;
            bester = static_cast<int>(i);
        }
    }
    return bester;
}

bool schiebeAnker(QPainterPath& pfad, int anker, QPointF ziel)
{
    const std::vector<Anker> alle = alleAnker(pfad);
    const bool zu = geschlossen(pfad, alle);
    const int zahl = static_cast<int>(alle.size()) - (zu ? 1 : 0);
    if (anker < 0 || anker >= zahl) return false;

    const auto schiebe = [&pfad](const Anker& a, QPointF schritt) {
        verschiebe(pfad, a.element, schritt);
        // der zweite Griff der Kurve, die hier endet
        if (a.kurvenEnde) verschiebe(pfad, a.element - 1, schritt);
        // der erste Griff der Kurve, die hier beginnt
        if (a.element + 1 < pfad.elementCount() && pfad.elementAt(a.element + 1).type == QPainterPath::CurveToElement)
            verschiebe(pfad, a.element + 1, schritt);
    };
    const Anker& a = alle[static_cast<std::size_t>(anker)];
    const QPointF schritt = ziel - ort(pfad, a.element);
    schiebe(a, schritt);
    // im geschlossenen Zug liegt das Ende auf dem Anfang: es geht mit
    if (zu && anker == 0) schiebe(alle.back(), schritt);
    return true;
}

}  // namespace lumi::shape
