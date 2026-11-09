/**
 * @file Tastatur.cpp
 * @brief Loop: der Block *Keyboard* (bis S175 in `src/loop/HammerSaite.cpp`)
 */
#include "loop/Tastatur.hpp"

#include <QJsonArray>

#include <algorithm>
#include <initializer_list>

namespace lumi::loop
{

bool pedalErlaubt(PedalPlatz platz, PedalArt art)
{
    if (art == PedalArt::Keins) return true;
    switch (platz)
    {
        case PedalPlatz::Links: return art == PedalArt::UnaCorda || art == PedalArt::Leise;
        case PedalPlatz::Mitte: return art == PedalArt::Sostenuto || art == PedalArt::Moderator || art == PedalArt::BassSustain;
        case PedalPlatz::Rechts: return art == PedalArt::Sustain;
    }
    return false;
}

QLatin1String pedalName(PedalArt art)
{
    switch (art)
    {
        case PedalArt::UnaCorda: return QLatin1String("unaCorda");
        case PedalArt::Leise: return QLatin1String("soft");
        case PedalArt::Sostenuto: return QLatin1String("sostenuto");
        case PedalArt::Moderator: return QLatin1String("practice");
        case PedalArt::Sustain: return QLatin1String("sustain");
        case PedalArt::BassSustain: return QLatin1String("bassSustain");
        case PedalArt::Keins: break;
    }
    return QLatin1String("none");
}

PedalArt pedalAusName(const QString& name)
{
    for (const PedalArt art : {PedalArt::UnaCorda, PedalArt::Leise, PedalArt::Sostenuto, PedalArt::Moderator, PedalArt::Sustain,
                               PedalArt::BassSustain})
        if (name == pedalName(art)) return art;
    return PedalArt::Keins;
}

void richteTastatur(Tastatur& t)
{
    t.bassSustainBis = std::clamp(t.bassSustainBis, kTastaturMin, kTastaturMax + 1);
    if (!pedalErlaubt(PedalPlatz::Links, t.links)) t.links = PedalArt::Keins;
    if (!pedalErlaubt(PedalPlatz::Mitte, t.mitte)) t.mitte = PedalArt::Keins;
    if (!pedalErlaubt(PedalPlatz::Rechts, t.rechts)) t.rechts = PedalArt::Keins;
}

QJsonObject tastaturAlsJson(const Tastatur& t)
{
    // KL4 a (Fassung 24): ohne `range` — den Umfang traegt die Tafel des Kerns
    return QJsonObject{{QStringLiteral("pedals"), QJsonObject{{QStringLiteral("left"), pedalName(t.links)},
                                                              {QStringLiteral("middle"), pedalName(t.mitte)},
                                                              {QStringLiteral("right"), pedalName(t.rechts)}}},
                       // seit Fassung 21 (FU2 a, aus IC F-2 a), immer geschrieben
                       {QStringLiteral("bassSustainBelow"), t.bassSustainBis}};
}

Tastatur tastaturAusJson(const QJsonObject& o)
{
    Tastatur t;
    t.bassSustainBis = o.value(QStringLiteral("bassSustainBelow")).toInt(t.bassSustainBis);
    const QJsonObject pedale = o.value(QStringLiteral("pedals")).toObject();
    // fehlt der Eintrag eines Platzes, gilt die Vorgabe; ein unbekannter heisst keins
    const auto pedal = [&pedale](const char* platz, PedalArt vorgabe) {
        const QJsonValue wert = pedale.value(QLatin1String(platz));
        return wert.isString() ? pedalAusName(wert.toString()) : vorgabe;
    };
    t.links = pedal("left", t.links);
    t.mitte = pedal("middle", t.mitte);
    t.rechts = pedal("right", t.rechts);
    richteTastatur(t);
    return t;
}

}  // namespace lumi::loop
