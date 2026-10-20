/**
 * @file Zeitspur.cpp
 * @brief Loop: die Spuren ueber der Zeit (Stufe 28a)
 */

#include "loop/Zeitspur.hpp"

#include <QJsonValue>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

double spurMin(Steuerspur spur)
{
    return spur == Steuerspur::Pitch ? -kPitchHalbtoeneMax : 0.0;
}

double spurMax(Steuerspur spur)
{
    switch (spur)
    {
    case Steuerspur::Vibrato: return kVibratoCentMax;
    case Steuerspur::Pitch: return kPitchHalbtoeneMax;
    case Steuerspur::Sustain:
    case Steuerspur::Sostenuto:
    case Steuerspur::Expression: break;
    }
    return 1.0;
}

bool istStufenspur(Steuerspur spur)
{
    return spur == Steuerspur::Sustain || spur == Steuerspur::Sostenuto;
}

QLatin1String steuerspurName(Steuerspur spur)
{
    switch (spur)
    {
    case Steuerspur::Sustain: return QLatin1String("sustain");
    case Steuerspur::Sostenuto: return QLatin1String("sostenuto");
    case Steuerspur::Expression: return QLatin1String("expression");
    case Steuerspur::Vibrato: return QLatin1String("vibrato");
    case Steuerspur::Pitch: return QLatin1String("pitch");
    }
    return QLatin1String("sustain");
}

bool Zeitspuren::leer() const
{
    return std::all_of(steuer.begin(), steuer.end(), [](const std::vector<SpurPunkt>& p) { return p.empty(); }) &&
           std::all_of(schalter.begin(), schalter.end(), [](const SchalterSpur& s) { return s.punkte.empty(); });
}

void richtePunkte(std::vector<SpurPunkt>& punkte, double min, double max, double ende)
{
    std::vector<SpurPunkt> gerichtet;
    gerichtet.reserve(punkte.size());
    for (SpurPunkt p : punkte)
    {
        if (!std::isfinite(p.stelle)) continue;
        if (ende > 0.0)
        {
            // im Muster: von 0 bis vor das Ende, wie Noten
            p.stelle = std::max(0.0, p.stelle);
            if (p.stelle >= ende) continue;
        }
        p.wert = std::isfinite(p.wert) ? std::clamp(p.wert, min, max) : min;
        gerichtet.push_back(p);
    }
    std::stable_sort(gerichtet.begin(), gerichtet.end(),
                     [](const SpurPunkt& a, const SpurPunkt& b) { return a.stelle < b.stelle; });
    // je Stelle ein Punkt: der spaetere gilt (wie beim Bend)
    std::vector<SpurPunkt> einzeln;
    for (const SpurPunkt& p : gerichtet)
    {
        if (!einzeln.empty() && einzeln.back().stelle == p.stelle)
            einzeln.back() = p;
        else
            einzeln.push_back(p);
    }
    if (einzeln.size() > static_cast<std::size_t>(kZeitspurPunkteMax))
        einzeln.resize(static_cast<std::size_t>(kZeitspurPunkteMax));
    punkte = std::move(einzeln);
}

void richteZeitspuren(Zeitspuren& spuren, double schritte)
{
    for (const Steuerspur spur : kAlleSteuerspuren) richtePunkte(spuren.von(spur), spurMin(spur), spurMax(spur), schritte);
    std::vector<SchalterSpur> gerichtet;
    for (SchalterSpur& s : spuren.schalter)
    {
        s.art = s.art.trimmed();
        if (s.art.isEmpty()) continue;
        if (std::any_of(gerichtet.begin(), gerichtet.end(), [&s](const SchalterSpur& g) { return g.art == s.art; }))
            continue;  // je Name bleibt der erste
        richtePunkte(s.punkte, 0.0, 1.0, schritte);
        for (SpurPunkt& p : s.punkte) p.wert = p.wert >= 0.5 ? 1.0 : 0.0;
        gerichtet.push_back(std::move(s));
    }
    // nach dem Namen — so steht es auch in der Datei (ein JSON-Objekt sortiert seine Schluessel)
    std::sort(gerichtet.begin(), gerichtet.end(), [](const SchalterSpur& a, const SchalterSpur& b) { return a.art < b.art; });
    spuren.schalter = std::move(gerichtet);
}

double stufeBei(const std::vector<SpurPunkt>& punkte, double stelle, double vorher)
{
    double wert = vorher;
    for (const SpurPunkt& p : punkte)
    {
        if (p.stelle > stelle) break;
        wert = p.wert;
    }
    return wert;
}

double kurveBei(const std::vector<SpurPunkt>& punkte, double stelle, double vorgabe)
{
    if (punkte.empty()) return vorgabe;
    if (stelle <= punkte.front().stelle) return punkte.front().wert;
    for (std::size_t i = 1; i < punkte.size(); ++i)
    {
        const SpurPunkt& a = punkte[i - 1];
        const SpurPunkt& b = punkte[i];
        if (stelle <= b.stelle)
            return b.stelle > a.stelle ? a.wert + (b.wert - a.wert) * (stelle - a.stelle) / (b.stelle - a.stelle) : b.wert;
    }
    return punkte.back().wert;
}

QJsonArray punkteAlsJson(const std::vector<SpurPunkt>& punkte)
{
    QJsonArray werte;
    for (const SpurPunkt& p : punkte) werte.append(QJsonArray{p.stelle, p.wert});
    return werte;
}

std::vector<SpurPunkt> punkteAusJson(const QJsonArray& werte)
{
    std::vector<SpurPunkt> punkte;
    for (const QJsonValue& v : werte)
    {
        const QJsonArray paar = v.toArray();
        if (paar.size() < 2) continue;
        punkte.push_back(SpurPunkt{paar[0].toDouble(0.0), paar[1].toDouble(0.0)});
    }
    return punkte;
}

QJsonObject zeitspurenAlsJson(const Zeitspuren& spuren)
{
    QJsonObject o;
    // immer alle fuenf (Datei-Felder immer schreiben): eine leere Spur ist die Vorgabe
    for (const Steuerspur spur : kAlleSteuerspuren) o[steuerspurName(spur)] = punkteAlsJson(spuren.von(spur));
    QJsonObject schalter;
    for (const SchalterSpur& s : spuren.schalter) schalter[s.art] = punkteAlsJson(s.punkte);
    o[QStringLiteral("switches")] = schalter;
    return o;
}

Zeitspuren zeitspurenAusJson(const QJsonObject& o)
{
    Zeitspuren spuren;
    for (const Steuerspur spur : kAlleSteuerspuren) spuren.von(spur) = punkteAusJson(o.value(steuerspurName(spur)).toArray());
    const QJsonObject schalter = o.value(QStringLiteral("switches")).toObject();
    for (auto it = schalter.begin(); it != schalter.end(); ++it)
        spuren.schalter.push_back(SchalterSpur{it.key(), punkteAusJson(it.value().toArray())});
    return spuren;
}

}  // namespace lumi::loop
