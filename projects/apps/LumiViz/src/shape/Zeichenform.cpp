/**
 * @file Zeichenform.cpp
 * @brief Klang aus einer Form: eine Zeichnung zum Zeichnen aufbereitet (s. Header)
 */

#include "shape/Zeichenform.hpp"

#include <QColor>

#include <algorithm>
#include <cmath>
#include <limits>

namespace lumi::shape
{

namespace
{

constexpr double kPi = 3.14159265358979323846;

void sorgeFuerReihe(ZeichenKontur& kontur)
{
    if (kontur.hatReihe) return;
    kontur.reihe = reihe(kontur.punkte);
    kontur.hatReihe = true;
}

}  // namespace

Zeichenform zeichenform(const Zeichnung& zeichnung)
{
    Zeichenform form;
    double links = std::numeric_limits<double>::max();
    double rechts = std::numeric_limits<double>::lowest();
    double unten = std::numeric_limits<double>::max();
    double oben = std::numeric_limits<double>::lowest();
    for (const Kontur& k : zeichnung.konturen)
    {
        ZeichenKontur z;
        z.punkte = abtasten(k.pfad);
        if (z.punkte.empty()) continue;
        z.kennung = k.kennung;
        z.gewaehlt = k.gewaehlt;
        const QColor farbe(k.farbe);
        if (farbe.isValid())
        {
            z.rot = static_cast<float>(farbe.redF());
            z.gruen = static_cast<float>(farbe.greenF());
            z.blau = static_cast<float>(farbe.blueF());
        }
        for (const Punkt& p : z.punkte)
        {
            links = std::min(links, p.real());
            rechts = std::max(rechts, p.real());
            unten = std::min(unten, p.imag());
            oben = std::max(oben, p.imag());
        }
        form.konturen.push_back(std::move(z));
    }
    if (form.konturen.empty()) return form;

    // alle gemeinsam: die Mitte des Rahmens in den Nullpunkt, die laengere Seite auf −1 … 1
    const Punkt mitte{(links + rechts) * 0.5, (unten + oben) * 0.5};
    const double halb = std::max(rechts - links, oben - unten) * 0.5;
    const double mass = halb > 0.0 ? 1.0 / halb : 1.0;
    for (ZeichenKontur& z : form.konturen)
        for (Punkt& p : z.punkte) p = (p - mitte) * mass;
    return form;
}

std::vector<std::size_t> gemeinteKonturen(const Zeichenform& form, int contour)
{
    std::vector<std::size_t> plaetze;
    for (std::size_t i = 0; i < form.konturen.size(); ++i)
    {
        const ZeichenKontur& z = form.konturen[i];
        if (contour == 0 ? z.gewaehlt : z.kennung == contour) plaetze.push_back(i);
    }
    return plaetze;
}

const std::vector<Punkt>& umrissVon(ZeichenKontur& kontur, double harmonische, int punkte)
{
    if (kontur.umrissHarmonische == harmonische && static_cast<int>(kontur.umriss.size()) == punkte)
        return kontur.umriss;
    sorgeFuerReihe(kontur);
    kontur.umriss = umriss(kontur.reihe, harmonische, punkte, /*mitMitte=*/true);
    kontur.umrissHarmonische = harmonische;
    return kontur.umriss;
}

Zeigerbild zeigerbild(ZeichenKontur& kontur, double umlauf, double harmonische)
{
    sorgeFuerReihe(kontur);
    const Reihe& r = kontur.reihe;
    Zeigerbild bild;
    Punkt ort = r.mitte;
    bild.arme.push_back(ort);
    const Punkt dreh = std::polar(1.0, 2.0 * kPi * umlauf);
    Punkt zeiger{1.0, 0.0};
    for (int k = 1; k <= r.harmonische(); ++k)
    {
        const double g = gewicht(harmonische, k);
        if (g <= 0.0) break;
        zeiger *= dreh;
        const auto platz = static_cast<std::size_t>(k - 1);
        for (const Punkt& arm : {g * r.plus[platz] * zeiger, g * r.minus[platz] * std::conj(zeiger)})
        {
            bild.kreise.push_back(Kreis{ort, std::abs(arm)});
            ort += arm;
            bild.arme.push_back(ort);
        }
    }
    return bild;
}

}  // namespace lumi::shape
