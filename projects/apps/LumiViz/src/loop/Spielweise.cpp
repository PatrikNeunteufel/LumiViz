/**
 * @file Spielweise.cpp
 * @brief Loop: Strum und die Artikulationen einer Note (Stufe 24a)
 */

#include "loop/Spielweise.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

int handStellen(HandRaster raster, int schlaegeJeTakt, int schritteJeSchlag)
{
    const int schlaege = std::max(1, schlaegeJeTakt);
    switch (raster)
    {
    case HandRaster::Schlag:
        return schlaege;
    case HandRaster::Schritt:
        return schlaege * std::max(1, schritteJeSchlag);
    case HandRaster::HalberSchlag:
        break;
    }
    return 2 * schlaege;
}

int handStelleVon(HandRaster raster, int schritt, int schlaegeJeTakt, int schritteJeSchlag)
{
    const int schritte = std::max(1, schritteJeSchlag);
    const int jeTakt = std::max(1, schlaegeJeTakt) * schritte;
    const int imTakt = ((schritt % jeTakt) + jeTakt) % jeTakt;
    switch (raster)
    {
    case HandRaster::Schlag:
        return imTakt / schritte;
    case HandRaster::Schritt:
        return imTakt;
    case HandRaster::HalberSchlag:
        break;
    }
    // ganzzahlig abgerundet: bei einer ungeraden Zahl von Schritten je Schlag
    // (Triolen) liegt die halbe Stelle zwischen zwei Schritten — es gilt die vorige
    return imTakt * 2 / schritte;
}

void richteStrum(Strum& strum, int stellen)
{
    strum.spreadMs = std::isfinite(strum.spreadMs) ? std::clamp(strum.spreadMs, 0.0, kSpreadMaxMs) : kSpreadVorgabeMs;
    if (strum.raster != HandRaster::Schlag && strum.raster != HandRaster::Schritt) strum.raster = HandRaster::HalberSchlag;
    const auto grenze = static_cast<std::size_t>(std::clamp(stellen, 0, kHandStellenMax));
    if (strum.hand.size() > grenze) strum.hand.resize(grenze);
    for (Richtung& richtung : strum.hand)
        if (richtung != Richtung::Auf) richtung = Richtung::Ab;
    // hinten weg, was ohnehin abwechselt: ein Muster hat genau eine Schreibweise
    while (!strum.hand.empty())
    {
        const int letzte = static_cast<int>(strum.hand.size()) - 1;
        const Richtung abwechselnd = letzte % 2 == 0 ? Richtung::Ab : Richtung::Auf;
        if (strum.hand.back() != abwechselnd) break;
        strum.hand.pop_back();
    }
}

QLatin1String handRasterName(HandRaster raster)
{
    switch (raster)
    {
    case HandRaster::Schlag:
        return QLatin1String("beat");
    case HandRaster::Schritt:
        return QLatin1String("step");
    case HandRaster::HalberSchlag:
        break;
    }
    return QLatin1String("halfBeat");
}

HandRaster handRasterAusName(const QString& name)
{
    if (name == QLatin1String("beat")) return HandRaster::Schlag;
    if (name == QLatin1String("step")) return HandRaster::Schritt;
    return HandRaster::HalberSchlag;
}

QString handAlsText(const std::vector<Richtung>& hand)
{
    QString text;
    text.reserve(static_cast<qsizetype>(hand.size()));
    for (const Richtung richtung : hand) text.append(richtung == Richtung::Auf ? QLatin1Char('U') : QLatin1Char('D'));
    return text;
}

std::vector<Richtung> handAusText(const QString& text)
{
    std::vector<Richtung> hand;
    hand.reserve(static_cast<std::size_t>(text.size()));
    for (qsizetype i = 0; i < text.size(); ++i)
    {
        const QChar zeichen = text.at(i).toUpper();
        if (zeichen == QLatin1Char('D'))
            hand.push_back(Richtung::Ab);
        else if (zeichen == QLatin1Char('U'))
            hand.push_back(Richtung::Auf);
        else
            hand.push_back(i % 2 == 0 ? Richtung::Ab : Richtung::Auf);
    }
    return hand;
}

QJsonObject strumAlsJson(const Strum& strum)
{
    QJsonObject o{{QStringLiteral("spread"), strum.spreadMs}, {QStringLiteral("hand"), handRasterName(strum.raster)}};
    if (!strum.hand.empty()) o[QStringLiteral("pattern")] = handAlsText(strum.hand);
    return o;
}

Strum strumAusJson(const QJsonObject& o)
{
    Strum strum;
    strum.spreadMs = o.value(QStringLiteral("spread")).toDouble(strum.spreadMs);
    strum.raster = handRasterAusName(o.value(QStringLiteral("hand")).toString());
    strum.hand = handAusText(o.value(QStringLiteral("pattern")).toString());
    return strum;
}

QLatin1String anschlagartName(Anschlagart art)
{
    switch (art)
    {
    case Anschlagart::Ab:
        return QLatin1String("down");
    case Anschlagart::Auf:
        return QLatin1String("up");
    case Anschlagart::Zugleich:
        return QLatin1String("together");
    case Anschlagart::Hand:
        break;
    }
    return QLatin1String("hand");
}

Anschlagart anschlagartAusName(const QString& name)
{
    if (name == QLatin1String("down")) return Anschlagart::Ab;
    if (name == QLatin1String("up")) return Anschlagart::Auf;
    if (name == QLatin1String("together")) return Anschlagart::Zugleich;
    return Anschlagart::Hand;
}

void richteBend(std::vector<BendPunkt>& bend)
{
    for (BendPunkt& p : bend)
    {
        p.zeit = std::isfinite(p.zeit) ? std::clamp(p.zeit, 0.0, 1.0) : 0.0;
        const double h = std::isfinite(p.halbtoene) ? std::clamp(p.halbtoene, 0.0, kBendHalbtoeneMax) : 0.0;
        p.halbtoene = std::round(h / kBendRaster) * kBendRaster;
    }
    std::stable_sort(bend.begin(), bend.end(), [](const BendPunkt& a, const BendPunkt& b) { return a.zeit < b.zeit; });
    // zweimal dieselbe Zeit: der spaetere gilt — der Sprung (etwa ein Pre-bend, der am Anfang steht)
    std::vector<BendPunkt> gerichtet;
    for (const BendPunkt& p : bend)
    {
        if (!gerichtet.empty() && gerichtet.back().zeit == p.zeit)
            gerichtet.back() = p;
        else
            gerichtet.push_back(p);
    }
    if (gerichtet.size() > static_cast<std::size_t>(kBendPunkteMax)) gerichtet.resize(static_cast<std::size_t>(kBendPunkteMax));
    if (std::all_of(gerichtet.begin(), gerichtet.end(), [](const BendPunkt& p) { return p.halbtoene == 0.0; }))
        gerichtet.clear();
    bend = std::move(gerichtet);
}

double bendBei(const std::vector<BendPunkt>& bend, double zeit)
{
    if (bend.empty()) return 0.0;
    if (zeit <= bend.front().zeit) return bend.front().halbtoene;
    for (std::size_t i = 1; i < bend.size(); ++i)
    {
        const BendPunkt& a = bend[i - 1];
        const BendPunkt& b = bend[i];
        if (zeit <= b.zeit)
            return b.zeit > a.zeit ? a.halbtoene + (b.halbtoene - a.halbtoene) * (zeit - a.zeit) / (b.zeit - a.zeit) : b.halbtoene;
    }
    return bend.back().halbtoene;
}

QJsonArray bendAlsJson(const std::vector<BendPunkt>& bend)
{
    QJsonArray werte;
    for (const BendPunkt& p : bend) werte.append(QJsonArray{p.zeit, p.halbtoene});
    return werte;
}

std::vector<BendPunkt> bendAusJson(const QJsonArray& werte)
{
    std::vector<BendPunkt> bend;
    for (const QJsonValue& v : werte)
    {
        const QJsonArray paar = v.toArray();
        if (paar.size() < 2) continue;
        bend.push_back(BendPunkt{paar[0].toDouble(0.0), paar[1].toDouble(0.0)});
    }
    return bend;
}

}  // namespace lumi::loop
